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
        _m_cf_alias_ = std::make_shared<cf_alias::CConfigAliases>( alias_file_path.data() );
        if( _m_cf_alias_.get() == NULL ) {
            std::string err = "Fail to create CConfigAliases about " + alias_file_path + ".";
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

std::shared_ptr<cf_alias::IAliasPVD> CAliasSearcherImpl::get_provider( const std::string& peer_app, const std::string& peer_pvd ) {
    std::shared_ptr<cf_alias::IAliasPVD> res;

    // TODO
    return res;
}


}   // alias