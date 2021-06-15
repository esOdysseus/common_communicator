/***
 * CAliasSearcher.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _ALIAS_SEARCHER_IMPLEMENTATION_H_
#define _ALIAS_SEARCHER_IMPLEMENTATION_H_

#include <string>
#include <memory>

#include <IAliasSearcher.h>
#include <CConfigAliases.h>


namespace alias {


    class CAliasSearcherImpl : public IAliasSearcher {
    public:
        CAliasSearcherImpl( const std::string& alias_file_path );

        ~CAliasSearcherImpl(void) override;

        // Open-API Lists
        std::shared_ptr<cf_alias::IAliasPVD> get_provider( const std::string& peer_app, const std::string& peer_pvd ) override;


        // Close-API Lists
        std::shared_ptr<cf_alias::CConfigAliases> get_config_alias( void ) {
            return _m_cf_alias_;
        }

    private:
        CAliasSearcherImpl(void) = delete;
        CAliasSearcherImpl( const CAliasSearcherImpl& inst ) = delete;
        CAliasSearcherImpl( CAliasSearcherImpl&& inst ) = delete;

    private:
        std::string _m_file_path_;

        std::shared_ptr<cf_alias::CConfigAliases> _m_cf_alias_;

    };


}

#endif // _ALIAS_SEARCHER_IMPLEMENTATION_H_