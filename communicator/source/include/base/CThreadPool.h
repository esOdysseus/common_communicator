/***
 * CThreadPool.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _C_THREAD_POOL_H_
#define _C_THREAD_POOL_H_

#include <map>
#include <list>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <functional>
#include <condition_variable>

#include <unistd.h>
#include <stdio.h>


/***
 * Thread-Pool class
 *
 * @desp It has a queue and multiple thread. So, the queue is shared among the threads.
 * @ARGS_TYPES It's support multiple argument type with heterogeneous types.
 *
 * @attention If you want to share a external-variable through a argument as ARGS_TYPES,
 *            then you have to define the argument-type with std::shared_ptr<XXX>.
 */
template <typename... ARGS_TYPES>
class CThreadPool {
public:
	static constexpr uint32_t CNT_DEF_THREAD = 3;

private:
    using RunFunc_Type = std::function<void(ARGS_TYPES... args)>;
    using IntFunc_Type = std::function<void(void)>;

protected:
    typedef enum class STATE : uint32_t {
        E_STATE_NOT_CREATED = 0,
        E_STATE_EXIT = 1,
        E_STATE_BUSY = 2,
        E_STATE_IDLE = 3
    } STATE;

    class OpenLOOPER{
    public:
        OpenLOOPER(CThreadPool* thread_pool) {
        #define MAX_WAIT_COUNT  100
            uint32_t cnt_wait = 0;
            if(thread_pool == NULL) {
                throw std::invalid_argument("thread_pool is NULL.");
            }

            clear();
            _p_thr_pool_ = thread_pool;
            _thr_h_ = std::thread(&OpenLOOPER::run_routin, this);

            while( _thr_h_.joinable() == false || _state_ <= STATE::E_STATE_EXIT ) { 
                // need it for waiting thread-initialization-done.
                if( cnt_wait >= MAX_WAIT_COUNT ) {
                    break;
                }
                usleep(500);    // wait 500 usec
                cnt_wait++;
            }

            if( cnt_wait >= MAX_WAIT_COUNT ) {  // max wait 50 msec.
                throw std::runtime_error("Can not create thread. (max waiting time: 50 msec");
            }
        #undef MAX_WAIT_COUNT
        }

        ~OpenLOOPER(void) {
            if(_state_ > STATE::E_STATE_EXIT) {
                if(_thr_h_.joinable() == true) {
                    std::unique_lock<std::mutex> quard(_mtx_wake_);

                    if( _state_ == STATE::E_STATE_IDLE ) {
                        wake(STATE::E_STATE_EXIT);
                        quard.unlock();
                        _thr_h_.join();
                    }
                }

                if( _thr_h_.joinable() == true ) {
                    _thr_h_.detach();
                }
            }
            clear();
        }

        std::thread::id get_id(void) {
            return _thr_h_.get_id();
        }

        STATE get_state(void) {
            return _state_;
        }

        bool trig_run_func(void) {
            bool ret = false;

            if( _state_ != STATE::E_STATE_IDLE ) {
                return ret;
            }

            std::unique_lock<std::mutex> quard(_mtx_wake_);

            if( _state_ == STATE::E_STATE_IDLE ) {
                // binding function
                if( (_run_func_ = _p_thr_pool_->pop_runfunc()) != NULL ) {
                    _is_job_ = true;

                    wake(STATE::E_STATE_BUSY);
                    ret = true;
                }
            }

            return ret;
        }

        bool try_run_func(RunFunc_Type func, ARGS_TYPES&&... args) {
            if( _state_ != STATE::E_STATE_IDLE ) {
                return false;
            }

            std::unique_lock<std::mutex> quard(_mtx_wake_);
            if( _state_ != STATE::E_STATE_IDLE ) {
                return false;
            }

            // binding function
            _run_func_ = std::bind( func, std::forward<ARGS_TYPES>(args)... );
            _is_job_ = true;

            wake(STATE::E_STATE_BUSY);
            return true;
        }

    private:
        void clear(void) {
            _state_ = STATE::E_STATE_NOT_CREATED;
            {
                std::unique_lock<std::mutex> quard(_mtx_wake_);
                _run_func_ = NULL;
                _is_job_ = false;
            }
        }

        void wake(STATE wanted) {
            _state_ = wanted;
            _wake_thread_.notify_one();
        }

        void run_routin(void) {
            std::thread::id thr_id = std::this_thread::get_id();

            try {
                std::unique_lock<std::mutex> wake_lock(_mtx_wake_);
                _state_ = STATE::E_STATE_BUSY;

                while( _state_ > STATE::E_STATE_EXIT ) {
                    switch(_state_) {
                    case STATE::E_STATE_IDLE:
                        // load new run_func from queue.
                        _run_func_ = _p_thr_pool_->pop_runfunc();
                        if(_run_func_ != NULL) {
                            _state_=STATE::E_STATE_BUSY;
                            _is_job_ = true;
                        }
                        else {
                            _p_thr_pool_->push_looper(thr_id);
                            _wake_thread_.wait(wake_lock);
                        }
                        break;
                    case STATE::E_STATE_BUSY:
                        {
                            while(_is_job_) {
                                _run_func_();   // do process

                                // load new run_func from queue.
                                _run_func_ = _p_thr_pool_->pop_runfunc();
                                if(_run_func_ != NULL) {
                                    continue;
                                }
                                _is_job_ = false;
                            }
                            _state_=STATE::E_STATE_IDLE;
                        }
                        break;
                    case STATE::E_STATE_EXIT:
                        break;
                    case STATE::E_STATE_NOT_CREATED:
                        break;
                    }
                }

                _state_ = STATE::E_STATE_EXIT;
            }
            catch( const std::exception& e ) {
                printf("OpenLOOPER::run_routin: %s\n", e.what());
            }
        }

    private:
        std::thread _thr_h_;

        CThreadPool* _p_thr_pool_;

        /** job mutex */
        IntFunc_Type _run_func_;

        bool _is_job_;

        /** state mutex */
        STATE _state_;

        std::condition_variable _wake_thread_;

        std::mutex _mtx_wake_;

    };

public:
    CThreadPool(uint32_t thread_max_count=CNT_DEF_THREAD){
        _thr_max_cnt_ = thread_max_count;
        _idles_list_.clear();

        for(uint32_t i=0; i < _thr_max_cnt_; i++) {
            create_loop();
        }
    }

    ~CThreadPool(void) {
        {
            std::unique_lock<std::mutex> guard(mtx_map_list);

            for( auto itor=_thr_map_.begin(); itor != _thr_map_.end(); ) {
                itor = _thr_map_.erase(itor);
            }

            _idles_list_.clear();
            _thr_map_.clear();
        }
        _thr_max_cnt_ = 0;
    }

    bool run_thread(RunFunc_Type func, bool run_immediatly, ARGS_TYPES... args) {
        bool result = false;
        std::shared_ptr<OpenLOOPER> looper;
        
        try{
            size_t count_idle_thread = 0;
            // push run_func to queue
            push_runfunc(func, std::forward<ARGS_TYPES>(args)...);

            // trig thread to wake-up.
            {
                std::unique_lock<std::mutex> guard(mtx_map_list);

                count_idle_thread = _idles_list_.size();
                if(_idles_list_.size() > 0) {
                    auto itor = _idles_list_.begin();
                    if( itor == _idles_list_.end() || _thr_map_.find(*itor) == _thr_map_.end() ) {
                        throw std::logic_error("There is a bug about thread-unsafe.");
                    }

                    looper = _thr_map_[*itor];
                }
                else if( run_immediatly == true ) {
                    looper = create_loop_no_locker();
                }

                if( looper.get() == NULL ) {
                    throw std::logic_error("Looper is not exist. NULL.");
                }

                _idles_list_.remove(looper->get_id());
                count_idle_thread = _idles_list_.size();
            }

            looper->trig_run_func();
            result = true;
        }
        catch ( const std::exception &e ) {
            printf("CThreadPool::run_thread: %s\n", e.what());
            result = false;
        }

        return result;
    }

private:
    std::shared_ptr<OpenLOOPER> create_loop(void) {
        std::shared_ptr<OpenLOOPER> looper;

        looper = std::make_shared<OpenLOOPER>(this);
        if(looper.get() == NULL) {
            throw std::runtime_error("OpenLOOPER class: memory-allocation is failed.");
        }

        std::unique_lock<std::mutex> guard(mtx_map_list);
        _thr_map_[looper->get_id()] = looper;
        return looper;
    }

    std::shared_ptr<OpenLOOPER> create_loop_no_locker(void) {
        std::shared_ptr<OpenLOOPER> looper;

        looper = std::make_shared<OpenLOOPER>(this);
        if(looper.get() == NULL) {
            throw std::runtime_error("OpenLOOPER class: memory-allocation is failed.");
        }

        _thr_map_[looper->get_id()] = looper;
        return looper;
    }

    void push_looper(std::thread::id thr_id) {
        std::unique_lock<std::mutex> guard(mtx_map_list);
        _idles_list_.push_back(thr_id);
    }

    void pop_looper(std::thread::id thr_id) {
        std::unique_lock<std::mutex> guard(mtx_map_list);
        _idles_list_.remove(thr_id);
    }

    void push_runfunc(RunFunc_Type func, ARGS_TYPES&&... args) {
        IntFunc_Type run_func = NULL;

        // binding function
        run_func = std::bind( func, std::forward<ARGS_TYPES>(args)... );

        // insert queue
        {
            std::unique_lock<std::mutex> lock_queue(mtx_queue);
            _q_runfunc_.push(run_func);
        }
    }

    IntFunc_Type pop_runfunc(void) {
        IntFunc_Type run_func = NULL;

        std::unique_lock<std::mutex> lock_queue(mtx_queue);
        if( _q_runfunc_.size() > 0 ) {
            run_func = _q_runfunc_.front();
            _q_runfunc_.pop();
        }

        return run_func;
    }

private:
    friend class OpenLOOPER;

    uint32_t _thr_max_cnt_;

    /** mutex for _idles_list_ & _thr_map_ */
    std::list<std::thread::id> _idles_list_;

    std::map< std::thread::id, std::shared_ptr<OpenLOOPER> > _thr_map_;

    std::mutex mtx_map_list;


    /** mutex for _q_runfunc_ */
    std::queue<IntFunc_Type> _q_runfunc_;

    std::mutex mtx_queue;

};


#endif // _C_THREAD_POOL_H_