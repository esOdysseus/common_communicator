/***
 * CConfigAliases.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _C_CONFIG_ALIASES_H_
#define _C_CONFIG_ALIASES_H_

#include <list>
#include <map>
#include <string>
#include <memory>
#include <cassert>

#include <json_manipulator.h>
#include <Enum_common.h>

namespace cf_alias {

    class CConfigAliases;

    typedef enum E_ERROR {
        E_NO_ERROR = 0,
        E_NOT_SUPPORTED_KEY = 1,
        E_NOT_SUPPORTED_PVD_TYPE = 2,
        E_NOT_SUPPORTED_APP_TYPE = 3,
        E_NOT_SUPPORTED_APP_PATH = 4
    }E_ERROR;

    typedef struct Sproperties {
        std::string type;
        std::string name;
        std::string where;
    } Sproperties;

    /***
     * Alias & Provider Interface.
     */
    class IAlias {
    public:
        IAlias(const char* alias_, enum_c::AliasType type_, std::shared_ptr<IAlias> parent_=std::shared_ptr<IAlias>());

        ~IAlias(void);

        // getter
        std::string name( void );

        std::string path( void );

        std::string path_parent( void );

        enum_c::AliasType alias_type( void );

    private:
        // setter
        void set_path_parent( IAlias& parent_ );

    private:
        std::string _m_name_;       // Current name.
        std::string _m_path_;       // Path of naming from resource to APP.
        enum_c::AliasType _m_type_;

    };

    class IAliasPVD : public IAlias {
    public:
        IAliasPVD(const char* alias_, const char* pvd_type_, std::shared_ptr<IAlias> parent_);

        ~IAliasPVD(void);

        // getter
        enum_c::ProviderType type( void );

        static std::string get_pvd_type(enum_c::ProviderType pvd_type);

    private:
        enum_c::ProviderType convert(std::string type_);

    private:
       enum_c::ProviderType _m_type_;

    };


    /***
     * Resource & APP & Provider & Service Aliasing Class.
     */
    class CAliasAPP : public IAlias {
    public:
        CAliasAPP( const char* alias_, const char* type_, std::shared_ptr<IAlias> parent_=std::shared_ptr<IAlias>() );

        virtual ~CAliasAPP( void );

        // getter
        enum_c::AppType type( void );

        std::string& get_where( void );

        std::shared_ptr<IAlias> get( std::string& alias_ );

    private:
        friend class CConfigAliases;

        // setter
        void set_where( std::string& where_ );

        void set( std::string& alias_, std::shared_ptr<IAlias> context_ );

        enum_c::AppType convert(std::string type_);

    private:
        enum_c::AppType _m_type_;
        std::string _m_where_;

        // string : app-name or provider-name
        // IAlias : app-instance or provider-instance
        std::map<std::string, std::shared_ptr<IAlias>> _mm_alias_;

    };

    class CAliasRSC : public CAliasAPP {
    public:
        CAliasRSC( const char* alias_, const char* type_ ) 
        : CAliasAPP(alias_, type_) {}

        ~CAliasRSC( void ) { 
            _m_self_.reset(); 
        }

        // getter
        std::shared_ptr<CAliasAPP> get_self( void ) {
            return _m_self_;
        }

    private:
        friend class CConfigAliases;

        // setter
        void set_self( std::shared_ptr<CAliasAPP> self_ ) {
            _m_self_ = self_;
        }

    private:
        std::shared_ptr<CAliasAPP> _m_self_;

    };

    class CAliasTrans : public IAliasPVD {
    public:
        CAliasTrans(const char* alias, const char* pvd_type, std::shared_ptr<IAlias> parent_=std::shared_ptr<IAlias>() );

        ~CAliasTrans(void);

        // getter
        std::string& get_ip( void );

        uint32_t get_mask( void );

        uint32_t get_port( void );

        // setter
        void set_ip( std::string ip_ );

        void set_mask( uint32_t mask_ );

        void set_port( uint32_t port_ );

    private:
        void clear( void );

    private:
        std::string _m_ip_;
        uint32_t _m_mask_;
        uint32_t _m_port_num_;
    };

    class CAliasService : public IAliasPVD {
    public:
        typedef struct Sreq_resp {
            uint32_t id;
        } Sreq_resp;

        typedef struct Spub_sub {
            uint32_t id;
            uint32_t grp_id;
        } Spub_sub;

    private:
        using ReqRespMapType = std::map<std::string /* name */, Sreq_resp>;
        using ReqNORespMapType = std::map<std::string /* name */, Sreq_resp>;
        using PubSubMapType = std::map<std::string /* name */, Spub_sub>;

    public:
        CAliasService(const char* alias, const char* pvd_type, std::shared_ptr<IAlias> parent_=std::shared_ptr<IAlias>() );

        ~CAliasService(void);

        // getter
        uint32_t get_svc_id( void );

        uint32_t get_inst_id( void );

    private:
        friend class CConfigAliases;

        // setter
        void set_svc_id( uint32_t svc_id_ );

        void set_inst_id( uint32_t inst_id_ );

        void push_reqresp_id( std::string& name_, uint32_t id_ );

        void push_reqNOresp_id( std::string& name_, uint32_t id_ );

        void push_pubsub_id( std::string& name_, uint32_t id_, uint32_t grp_id_ );

        void clear( void );

    public:
        uint32_t _m_svc_id_;
        uint32_t _m_inst_id_;

        ReqRespMapType _mm_req_resp_;
        ReqNORespMapType _mm_req_noresp_;
        PubSubMapType _mm_pub_sub_;

    };


    /***
     * Major Configuration-Aliasing Class.
     */
    class CConfigAliases {
    public:
        // for svc-pvd/XXX/provider-type
        static constexpr const char* UDP = "udp";
        static constexpr const char* UDP_UDS = "udp_uds";
        static constexpr const char* TCP = "tcp";
        static constexpr const char* TCP_UDS = "tcp_uds";
        static constexpr const char* VSOMEIP = "vsomeip";
        // for properties/type
        static constexpr const char* SINGLE = "single";
        static constexpr const char* MULTIPLE = "multi";
        // for properties/name
        static constexpr const char* SELF = "self";

        using PVDListType = std::list<std::shared_ptr<IAliasPVD>>;

    private:
        // provider_type : 'udp' , 'tcp' , 'vsomeip' , 'udp_uds' , 'tcp_uds'
        using PVDMapType = std::map< std::string /* provider_type */, PVDListType >;

        using RSCMapType = std::map< std::string /* rsc_name */, std::shared_ptr<CAliasRSC> >;
        using APPMapType = std::map< std::string /* app_path */, PVDMapType >;

    public:
        CConfigAliases(const char* config_path);

        ~CConfigAliases(void);

        PVDListType& get_providers(std::string type);

        PVDListType& get_providers(std::string app_path, std::string type);

    private:
        CConfigAliases(void) = delete;

        bool init(const std::string config_path);

        // resource builder
        bool append_rsc_alias( std::string& rsc_name, 
                               std::shared_ptr<json_mng::CMjson> obj_value );

        std::shared_ptr<CAliasAPP> make_app_alias( std::string& name_, 
                                                   std::shared_ptr<json_mng::CMjson>& obj_, 
                                                   std::shared_ptr<CAliasAPP>& self_,
                                                   std::shared_ptr<CAliasAPP> parent_=std::shared_ptr<CAliasAPP>() );

        std::shared_ptr<Sproperties> get_property( std::shared_ptr<json_mng::CMjson>& obj_ );

        // provider builder 
        bool append_pvd_alias( json_mng::MemberIterator& itr, std::shared_ptr<CAliasAPP>& app_ );

        void append_pvd_context( std::string&& app_path, std::string& pvd_type, std::shared_ptr<IAliasPVD>& context );

        std::shared_ptr<IAliasPVD> make_pvd_alias(std::string alias, 
                                                  std::shared_ptr<json_mng::CMjson> obj_value, 
                                                  std::string &pvd_type_str,
                                                  std::shared_ptr<CAliasAPP>& parent_);

        std::shared_ptr<CAliasTrans> make_pvd_trans(std::string alias, std::string pvd_type, 
                                                    std::shared_ptr<json_mng::CMjson> &obj_addr,
                                                    std::shared_ptr<CAliasAPP>& parent_);

        std::shared_ptr<CAliasService> make_pvd_service(std::string alias, std::string pvd_type, 
                                                        std::shared_ptr<json_mng::CMjson> &obj_addr,
                                                        std::shared_ptr<CAliasAPP>& parent_);

        void set_pvd_service_with_function( std::shared_ptr<CAliasService>& pvd_, 
                                            std::shared_ptr<json_mng::CMjson> &obj_ );

    private:
        // // valid 'value' assignment of configuration.
        static const char* _ma_pvd_types_[];

        // provider_type : 'udp' , 'tcp' , 'vsomeip' , 'udp_uds' , 'tcp_uds'
        PVDMapType _mm_pvds_;

        // store resource-instance per resource-name
        RSCMapType _mm_rscs_;

        // store mapper for provider-instances per app-path
        // app-path : 'APP-01/sub01/sub-app'
        APPMapType _mm_pvds_map_;

        bool _m_f_ready_;

    };


}   // cf_alias

#endif // _C_CONFIG_ALIASES_H_