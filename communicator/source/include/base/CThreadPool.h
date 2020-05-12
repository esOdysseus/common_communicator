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
#include <cassert>
#include <memory>
#include <thread>
#include <functional>
#include <condition_variable>

#include <unistd.h>

#include <logger.h>

template <typename... ARGS_TYPES>
class CThreadPool {
public:
	static constexpr uint32_t Default_thr_cnt = 3;

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
            assert(thread_pool != NULL);

            clear();
            _p_thr_pool_ = thread_pool;
            _thr_h_ = std::thread(&OpenLOOPER::run_routin, this);

            usleep(50000);
            if(_thr_h_.joinable() == false || _state_ <= STATE::E_STATE_EXIT) {
                throw std::runtime_error("Can not create thread.");
            }
        }

        ~OpenLOOPER(void) {
            if(_state_ > STATE::E_STATE_EXIT) {
                assert(_thr_h_.joinable() == true);

                std::unique_lock<std::mutex> quard(_mtx_wake_);

                if( _state_ == STATE::E_STATE_IDLE ) {
                    wake(STATE::E_STATE_EXIT);
                    _thr_h_.join();
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
            std::thread::id thr_id = _thr_h_.get_id();

            _state_ = wanted;
            if(_state_ != STATE::E_STATE_IDLE) {
                _p_thr_pool_->_idles_list_.remove(thr_id);
            }
            _wake_thread_.notify_one();
        }

        void run_routin(void) {
            std::thread::id thr_id = std::this_thread::get_id();
            std::unique_lock<std::mutex> wake_lock(_mtx_wake_);
            _state_ = STATE::E_STATE_BUSY;

            while( _state_ > STATE::E_STATE_EXIT ) {
                switch(_state_) {
                case STATE::E_STATE_IDLE:
                    LOGD("State is E_STATE_IDLE.(0x%X)", thr_id);

                    _p_thr_pool_->_idles_list_.push_back(thr_id);

                    // load new run_func from queue.
                    _run_func_ = _p_thr_pool_->pop_runfunc();
                    if(_run_func_ != NULL) {
                        _state_=STATE::E_STATE_BUSY;
                        _is_job_ = true;
                        _p_thr_pool_->_idles_list_.remove(thr_id);
                    }
                    else {
                        _wake_thread_.wait(wake_lock);
                    }
                    break;
                case STATE::E_STATE_BUSY:
                    LOGD("State is E_STATE_BUSY.(0x%X)", thr_id);
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
                    LOGD("State is E_STATE_EXIT.(0x%X)", thr_id);
                    break;
                case STATE::E_STATE_NOT_CREATED:
                    LOGERR("State is E_STATE_NOT_CREATED.(0x%X)", thr_id);
                    break;
                }
            }

            _state_ = STATE::E_STATE_EXIT;
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
    CThreadPool(uint32_t thread_max_count=Default_thr_cnt){
    	std::shared_ptr<OpenLOOPER> looper;

    	try {
            _thr_max_cnt_ = thread_max_count;
            _idles_list_.clear();

            for(uint32_t i=0; i < _thr_max_cnt_; i++) {
                looper.reset();
                looper = std::make_shared<OpenLOOPER>(this);
                assert(looper.get() != NULL);
                _thr_map_[looper->get_id()] = looper;

                LOGD("Create Thread-ID(0x%X) is successful.", looper->get_id());
            }
    	}
    	catch( const std::exception &e ) {
    	    LOGERR("%s", e.what());
    	    throw;
    	}
    }

    ~CThreadPool(void) {
        for( auto itor=_thr_map_.begin(); itor != _thr_map_.end(); itor++ ) {
            LOGD("Destroy Thread-ID(0x%X) in ThreadPool.", itor->first);
            itor->second.reset();
        }

        _idles_list_.clear();
        _thr_map_.clear();
        _thr_max_cnt_ = 0;
    }

    bool run_thread(RunFunc_Type func, ARGS_TYPES... args) {
        std::shared_ptr<OpenLOOPER> looper;
        
        try{
            // push run_func to queue
            push_runfunc(func, std::forward<ARGS_TYPES>(args)...);

            // trig thread to wake-up.
            LOGD("idle-thread cnt = %d", _idles_list_.size());
            if(_idles_list_.size() > 0) {
                auto itor = _idles_list_.begin();
                if( itor != _idles_list_.end() && _thr_map_.find(*itor) != _thr_map_.end() ) {
                    looper = _thr_map_[*itor];
                    looper->trig_run_func();
                }
            }
            LOGI("idle-thread cnt = %d", _idles_list_.size());

            return true;
        }
        catch ( const std::exception &e ) {
            LOGERR("%s", e.what());
        }

        return false;
    }

private:
    void push_runfunc(RunFunc_Type func, ARGS_TYPES&&... args) {
        IntFunc_Type run_func = NULL;

        // binding function
        run_func = std::bind( func, std::forward<ARGS_TYPES>(args)... );

        // insert queue
        {
            std::unique_lock<std::mutex> lock_queue(mtx_queue);
            _q_runfunc_.push(run_func);
        }
        LOGI("queue size=%d", _q_runfunc_.size());
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

    std::list<std::thread::id> _idles_list_;

    std::map< std::thread::id, std::shared_ptr<OpenLOOPER> > _thr_map_;

    /** mutex for _q_runfunc_ */
    std::queue<IntFunc_Type> _q_runfunc_;

    std::mutex mtx_queue;

};

#endif // _C_THREAD_POOL_H_