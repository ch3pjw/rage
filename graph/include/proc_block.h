#pragma once
#include "error.h"
#include "loader.h"
#include "time_series.h"

typedef struct rage_ProcBlock rage_ProcBlock;
typedef RAGE_OR_ERROR(rage_ProcBlock *) rage_NewProcBlock;
typedef struct rage_Harness rage_Harness;
typedef RAGE_OR_ERROR(rage_Harness *) rage_MountResult;

rage_NewProcBlock rage_proc_block_new(uint32_t * sample_rate);
void rage_proc_block_free(rage_ProcBlock * pb);

rage_Error rage_proc_block_start(rage_ProcBlock * pb);
rage_Error rage_proc_block_stop(rage_ProcBlock * pb);

rage_MountResult rage_proc_block_mount(
    rage_ProcBlock * pb, rage_Element * elem, rage_TimeSeries * controls,
    char const * name);
void rage_proc_block_unmount(rage_Harness * harness);

rage_Finaliser * rage_harness_set_time_series(
    rage_Harness * const harness,
    uint32_t const series_idx,
    rage_TimeSeries const * const new_controls);

void rage_proc_block_set_transport_state(rage_ProcBlock * pb, rage_TransportState state);
