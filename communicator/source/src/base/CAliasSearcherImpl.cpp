/***
 * IAliasSearcher.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */

#include <logger.h>
#include <CAliasSearcherImpl.h>

namespace alias {


CAliasSearcherImpl::CAliasSearcherImpl( const std::string& alias_file_path )
: _m_file_path_(alias_file_path) {
    try {
        _m_cf_alias_ = std::make_shared<cf_alias::CConfigAliases>( _m_file_path_.data() );
        if( _m_cf_alias_.get() == NULL ) {
            std::string err = "Fail to create CConfigAliases about " + _m_file_path_ + ".";
            throw std::runtime_error(err);
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

CAliasSearcherImpl::~CAliasSearcherImpl(void) {
    _m_cf_alias_.reset();
    _m_file_path_.clear();
}

/// Search my-provider list that is created by my application.
std::map<std::string, IAliasSearcher::TPvdList> CAliasSearcherImpl::get_mypvds( const std::string& my_app ) {
    try {
        return _m_cf_alias_->get_providers( my_app );
    }
    catch ( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

std::shared_ptr<cf_alias::IAliasPVD> CAliasSearcherImpl::get_peer_provider( const std::string& peer_app, const std::string& peer_pvd ) {
    try {
        return _m_cf_alias_->get_provider( peer_app, peer_pvd );
    }
    catch ( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}

IAliasSearcher::TPvdList CAliasSearcherImpl::get_mypvds_sendable( const std::string& peer_app, const std::string& peer_pvd ) {
    try {
        return _m_cf_alias_->get_connected_provider_to_peer( peer_app, peer_pvd );
    }
    catch ( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
}


}   // alias