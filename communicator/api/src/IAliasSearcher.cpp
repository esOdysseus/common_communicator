/***
 * IAliasSearcher.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */

#include <map>
#include <memory>

#include <logger.h>
#include <IAliasSearcher.h>
#include <CAliasSearcherImpl.h>

namespace alias {


using TmASearcher = std::map<std::string /*alias_path*/, std::shared_ptr<CAliasSearcherImpl>>;
static TmASearcher  _gm_aliases_;

static std::shared_ptr<CAliasSearcherImpl> update_get_alias_searcher(const std::string& file_path) {
    TmASearcher::iterator iter;

    try {
        iter = _gm_aliases_.find(file_path);

        if( iter == _gm_aliases_.end() ) {
            auto res = _gm_aliases_.insert( {file_path, std::make_shared<CAliasSearcherImpl>(file_path)} );
            if( res.second != true ) {
                std::string err = "Key is duplicated. (" + file_path + ")";
                throw std::logic_error(err);
            }
            iter = res.first;
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }
    return iter->second;
}

std::shared_ptr<IAliasSearcher> IAliasSearcher::get_instance( const std::string& alias_file_path ) {
    std::shared_ptr<IAliasSearcher> res;

    try {
        auto searcher = update_get_alias_searcher( alias_file_path );
        if ( searcher.get() != NULL ) {
            return searcher;
        }
    }
    catch( const std::exception& e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return res;
}


}   // alias