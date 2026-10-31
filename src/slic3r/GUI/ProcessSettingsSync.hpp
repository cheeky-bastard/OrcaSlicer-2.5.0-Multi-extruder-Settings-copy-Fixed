#pragma once

#include "libslic3r/Config.hpp"
#include "libslic3r/PrintConfig.hpp"
#include <cstddef>
#include <memory>
#include <set>
#include <vector>

namespace Slic3r { namespace GUI {

// Return compatible active destinations; profile arrays may retain inactive trailing slots.
inline std::vector<int> process_tool_sync_targets(int count, int source,
                                                 const std::vector<int>& nozzles,
                                                 const std::vector<int>& drives,
                                                 NozzleVolumeType selected_nozzle)
{
    if (count < 2 || source < 0 || source >= count ||
        nozzles.size() < static_cast<std::size_t>(count) || drives.size() < static_cast<std::size_t>(count))
        return {};
    std::vector<int> targets;
    for (int target = 0; target < count; ++target) {
        if (target == source || drives[target] != drives[source])
            continue;
        if (nozzles[source] == nozzles[target] || nozzles[target] == nvtHybrid ||
            (nozzles[source] == nvtHybrid && selected_nozzle == nozzles[target]))
            targets.push_back(target);
    }
    return targets;
}

// Copy the selected page's process values, including saved/default values.
// The caller supplies page keys; printer, filament and routing keys are excluded here.
inline DynamicConfig process_tool_sync_changes(const DynamicPrintConfig& config,
                                               const t_config_option_keys& page_keys,
                                               int source, int target)
{
    DynamicConfig changes;
    if (source < 0 || target < 0 || source == target)
        return changes;
    for (const auto& key : page_keys) {
        if (print_options_with_variant.count(key) == 0 ||
            key == "print_extruder_id" || key == "print_extruder_variant")
            continue;
        const auto* option = dynamic_cast<const ConfigOptionVectorBase*>(config.option(key));
        if (!option || static_cast<std::size_t>(source) >= option->size() || static_cast<std::size_t>(target) >= option->size())
            continue;
        std::unique_ptr<ConfigOptionVectorBase> copied(static_cast<ConfigOptionVectorBase*>(option->clone()));
        copied->set_at(option, static_cast<std::size_t>(target), static_cast<std::size_t>(source));
        if (*option != *copied)
            changes.set_key_value(key, copied.release());
    }
    return changes;
}

}} // namespace Slic3r::GUI
