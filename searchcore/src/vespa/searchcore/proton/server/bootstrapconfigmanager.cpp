// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "bootstrapconfigmanager.h"
#include "bootstrapconfig.h"
#include <vespa/searchlib/common/tunefileinfo.hpp>

#include <vespa/log/log.h>
LOG_SETUP(".proton.server.bootstrapconfigmanager");

using namespace vespa::config::search;
using namespace config;
using document::DocumentTypeRepo;
using search::TuneFileDocumentDB;
using vespa::config::search::core::ProtonConfig;
using cloud::config::filedistribution::FiledistributorrpcConfig;
using document::DocumenttypesConfig;

namespace proton {

BootstrapConfigManager::BootstrapConfigManager(const vespalib::string & configId)
    : _pendingConfigSnapshot(),
      _configId(configId),
      _pendingConfigMutex()
{ }

BootstrapConfigManager::~BootstrapConfigManager() { }


const ConfigKeySet
BootstrapConfigManager::createConfigKeySet() const
{
    return ConfigKeySet().add<ProtonConfig>(_configId)
                         .add<DocumenttypesConfig>(_configId)
                         .add<FiledistributorrpcConfig>(_configId);
}

void
BootstrapConfigManager::update(const ConfigSnapshot & snapshot)
{
    typedef BootstrapConfig::ProtonConfigSP ProtonConfigSP;
    typedef BootstrapConfig::DocumenttypesConfigSP DocumenttypesConfigSP;

    ProtonConfigSP newProtonConfig;
    BootstrapConfig::FiledistributorrpcConfigSP newFiledistRpcConfSP;
    TuneFileDocumentDB::SP newTuneFileDocumentDB;
    DocumenttypesConfigSP newDocumenttypesConfig;
    DocumentTypeRepo::SP newRepo;
    int64_t currentGen = -1;

    BootstrapConfig::SP current = _pendingConfigSnapshot;
    if (current.get() != nullptr) {
        newProtonConfig = current->getProtonConfigSP();
        newFiledistRpcConfSP = current->getFiledistributorrpcConfigSP();
        newTuneFileDocumentDB = current->getTuneFileDocumentDBSP();
        newDocumenttypesConfig = current->getDocumenttypesConfigSP();
        newRepo = current->getDocumentTypeRepoSP();
        currentGen = current->getGeneration();
    }

    if (snapshot.isChanged<ProtonConfig>(_configId, currentGen)) {
        LOG(spam, "Proton config is changed");
        std::unique_ptr<ProtonConfig> protonConfig = snapshot.getConfig<ProtonConfig>(_configId);
        TuneFileDocumentDB::SP tuneFileDocumentDB(new TuneFileDocumentDB);
        TuneFileDocumentDB &tune = *tuneFileDocumentDB;
        ProtonConfig &conf = *protonConfig;
        tune._index._indexing._write.setFromConfig<ProtonConfig::Indexing::Write>(conf.indexing.write.io);
        tune._index._indexing._read.setFromConfig<ProtonConfig::Indexing::Read>(conf.indexing.read.io);
        tune._attr._write.setFromConfig<ProtonConfig::Attribute::Write>(conf.attribute.write.io);
        tune._index._search._read.setFromConfig<ProtonConfig::Search, ProtonConfig::Search::Mmap>(conf.search.io, conf.search.mmap);
        tune._summary._write.setFromConfig<ProtonConfig::Summary::Write>(conf.summary.write.io);
        tune._summary._seqRead.setFromConfig<ProtonConfig::Summary::Read>(conf.summary.read.io);
        tune._summary._randRead.setFromConfig<ProtonConfig::Summary::Read, ProtonConfig::Summary::Read::Mmap>(conf.summary.read.io, conf.summary.read.mmap);

        newProtonConfig = ProtonConfigSP(protonConfig.release());
        newTuneFileDocumentDB = tuneFileDocumentDB;
    }

    if (snapshot.isChanged<FiledistributorrpcConfig>(_configId, currentGen)) {
        LOG(info, "Filedistributorrpc config is changed");
        auto p = snapshot.getConfig<FiledistributorrpcConfig>(_configId);
        newFiledistRpcConfSP = BootstrapConfig::FiledistributorrpcConfigSP(std::move(p));
    }

    if (snapshot.isChanged<DocumenttypesConfig>(_configId, currentGen)) {
        LOG(spam, "Documenttypes config is changed");
        std::unique_ptr<DocumenttypesConfig> documenttypesConfig = snapshot.getConfig<DocumenttypesConfig>(_configId);
        DocumentTypeRepo::SP repo(new DocumentTypeRepo(*documenttypesConfig));
        newDocumenttypesConfig = DocumenttypesConfigSP(documenttypesConfig.release());
        newRepo = repo;
    }
    assert(newProtonConfig.get() != nullptr);
    assert(newFiledistRpcConfSP.get() != nullptr);
    assert(newTuneFileDocumentDB.get() != nullptr);
    assert(newDocumenttypesConfig.get() != nullptr);
    assert(newRepo.get() != nullptr);

    auto newSnapshot(std::make_shared<BootstrapConfig>(snapshot.getGeneration(), newDocumenttypesConfig, newRepo,
                                                       newProtonConfig, newFiledistRpcConfSP, newTuneFileDocumentDB));

    assert(newSnapshot->valid());
    {
        std::lock_guard<std::mutex> lock(_pendingConfigMutex);
        _pendingConfigSnapshot = newSnapshot;
    }
}

} // namespace proton
