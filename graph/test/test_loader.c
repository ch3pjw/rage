#include <stdlib.h>
#include "loader.h"
#include "pdls.c"

static void new_stream_buffers(rage_Ports * ports, rage_ProcessRequirements requirements) {
    uint32_t i;
    for (i = 0; i < requirements.inputs.len; i++) {
        ports->inputs[i] = calloc(256, sizeof(float));
    }
    for (i = 0; i < requirements.outputs.len; i++) {
        ports->outputs[i] = calloc(256, sizeof(float));
    }
}

static void free_stream_buffers(rage_Ports * ports, rage_ProcessRequirements requirements) {
    uint32_t i;
    for (i = 0; i < requirements.inputs.len; i++) {
        free((void *) ports->inputs[i]);
    }
    for (i = 0; i < requirements.outputs.len; i++) {
        free((void *) ports->outputs[i]);
    }
}

typedef RAGE_ARRAY(rage_Interpolator *) rage_InterpolatorArray;
typedef RAGE_OR_ERROR(rage_InterpolatorArray) rage_Interpolators;

static rage_Interpolators new_interpolators(rage_Ports * ports, rage_ProcessRequirements requirements) {
    rage_InterpolatorArray interpolators;
    RAGE_ARRAY_INIT(&interpolators, requirements.controls.len, i) {
        rage_TimeSeries ts = rage_time_series_new(&requirements.controls.items[i]);
        rage_InitialisedInterpolator ii = rage_interpolator_new(
            &requirements.controls.items[i], &ts, 44100, 1);
        rage_time_series_free(ts);
        // FIXME: unwinding memory frees etc
        RAGE_EXTRACT_VALUE(rage_Interpolators, ii, interpolators.items[i])
        ports->controls[i] = rage_interpolator_get_view(interpolators.items[i], 0);
    }
    RAGE_SUCCEED(rage_Interpolators, interpolators)
}

static void free_interpolators(rage_InterpolatorArray interpolators, rage_ProcessRequirements requirements) {
    for (uint32_t i = 0; i < interpolators.len; i++) {
        rage_interpolator_free(&requirements.controls.items[i], interpolators.items[i]);
    }
}

rage_Error test() {
    // FIXME: Error handling (and memory in those cases)
    rage_ElementLoader * el = rage_element_loader_new();
    rage_ElementTypes element_type_names = rage_element_loader_list(el);
    for (unsigned i=0; i<element_type_names.len; i++) {
        rage_ElementTypeLoadResult et_ = rage_element_loader_load(
            el, element_type_names.items[i]);
        RAGE_EXTRACT_VALUE(rage_Error, et_, rage_ElementType * et)
        rage_Atom ** tups = generate_tuples(et->parameters);
        rage_ElementNewResult elem_ = rage_element_new(et, 44100, 256, tups);
        RAGE_EXTRACT_VALUE(rage_Error, elem_, rage_Element * elem)
        rage_Ports ports = rage_ports_new(&elem->requirements);
        new_stream_buffers(&ports, elem->requirements);
        rage_Interpolators ii = new_interpolators(&ports, elem->requirements);
        RAGE_EXTRACT_VALUE(rage_Error, ii, rage_InterpolatorArray interpolators);
        rage_element_process(elem, RAGE_TRANSPORT_ROLLING, &ports);
        free_interpolators(interpolators, elem->requirements);
        free_stream_buffers(&ports, elem->requirements);
        rage_ports_free(ports);
        rage_element_free(elem);
        free_tuples(et->parameters, tups);
        rage_element_loader_unload(el, et);
    }
    rage_element_loader_free(el);
    RAGE_OK
}
