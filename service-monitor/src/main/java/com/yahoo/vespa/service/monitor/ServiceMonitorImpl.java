// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.service.monitor;

import com.google.inject.Inject;
import com.yahoo.cloud.config.ConfigserverConfig;
import com.yahoo.config.model.api.SuperModelProvider;
import com.yahoo.config.provision.Zone;
import com.yahoo.jdisc.Metric;
import com.yahoo.jdisc.Timer;
import com.yahoo.vespa.applicationmodel.ApplicationInstance;
import com.yahoo.vespa.applicationmodel.ApplicationInstanceReference;
import com.yahoo.vespa.service.monitor.internal.ServiceMonitorMetrics;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;
import java.util.stream.Collectors;

public class ServiceMonitorImpl implements ServiceMonitor {
    private static final Logger logger = Logger.getLogger(ServiceMonitorImpl.class.getName());

    private final Zone zone;
    private final List<String> configServerHosts;
    private final SlobrokMonitorManager slobrokMonitorManager = new SlobrokMonitorManager();
    private final SuperModelListenerImpl superModelListener;

    @Inject
    public ServiceMonitorImpl(SuperModelProvider superModelProvider,
                              ConfigserverConfig configserverConfig,
                              Metric metric,
                              Timer timer) {
        this.zone = superModelProvider.getZone();
        this.configServerHosts = toConfigServerList(configserverConfig);
        ServiceMonitorMetrics metrics = new ServiceMonitorMetrics(metric, timer);
        this.superModelListener = new SuperModelListenerImpl(
                slobrokMonitorManager,
                metrics,
                new ModelGenerator());
        superModelListener.start(superModelProvider);
    }

    private List<String> toConfigServerList(ConfigserverConfig configserverConfig) {
        if (configserverConfig.multitenant()) {
            return configserverConfig.zookeeperserver().stream()
                    .map(server -> server.hostname())
                    .collect(Collectors.toList());
        }

        return Collections.emptyList();
    }

    @Override
    public Map<ApplicationInstanceReference,
            ApplicationInstance<ServiceMonitorStatus>> queryStatusOfAllApplicationInstances() {
        // If we ever need to optimize this method, then consider reusing ServiceModel snapshots
        // for up to X ms.
        ServiceModel serviceModel =
                superModelListener.createServiceModelSnapshot(zone, configServerHosts);
        return serviceModel.getAllApplicationInstances();
    }
}
