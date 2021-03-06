#include <stdlib.h>
// FIXME: these includes should be "systemy"
// This header should probably have a hash of the interface files or something
// dynamically embedded in it:
#include "element_impl.h"
#include "interpolation.h"

static rage_AtomDef const n_channels = {
    .type = RAGE_ATOM_INT,
    .name = "n_channels",
    .constraints = {.i = {.min = RAGE_JUST(1)}}
};

static rage_FieldDef const param_fields[] = {
    {.name = "channels", .type = &n_channels}
};

rage_TupleDef const param_tup = {
    .name = "amp",
    .description = "Amplifier",
    .default_value = NULL,
    .len = 1,
    .items = param_fields
};

rage_ParamDefList const init_params = {
    .len = 1,
    .items = &param_tup
};

static rage_AtomDef const gain_float = {
    .type = RAGE_ATOM_FLOAT, .name = "gain",
    .constraints = {.f = {.max = RAGE_JUST(5.0)}}
};

static rage_FieldDef const gain_fields[] = {
    {.name = "gain", .type = &gain_float}
};

static rage_Atom const gain_default[] = {
    {.f = 1.0}
};

static rage_TupleDef const gain_def = {
    .name = "gain",
    .description = "Loudness",
    .default_value = gain_default,
    .len = 1,
    .items = gain_fields
};

rage_NewProcessRequirements elem_describe_ports(rage_Atom ** params) {
    int const n_channels = params[0][0].i;
    rage_ProcessRequirements rval;
    rval.controls.len = 1;
    rval.controls.items = &gain_def;
    rage_StreamDef * stream_defs = calloc(n_channels, sizeof(rage_StreamDef));
    for (unsigned i = 0; i < n_channels; i++) {
        stream_defs[i] = RAGE_STREAM_AUDIO;
    }
    rval.inputs.len = n_channels;
    rval.inputs.items = stream_defs;
    rval.outputs.len = n_channels;
    rval.outputs.items = stream_defs;
    RAGE_SUCCEED(rage_NewProcessRequirements, rval)
}

void elem_free_port_description(rage_ProcessRequirements pr) {
    // FIXME: too const requiring cast?
    free((void *) pr.inputs.items);
}

typedef struct {
    unsigned n_channels;
    uint32_t period_size; // Doesn't everyone HAVE to do this?
} amp_data;

rage_NewElementState elem_new(
        uint32_t sample_rate, uint32_t period_size, rage_Atom ** params) {
    amp_data * ad = malloc(sizeof(amp_data));
    // Not sure I like way the indices tie up here
    ad->n_channels = params[0][0].i;
    ad->period_size = period_size;
    RAGE_SUCCEED(rage_NewElementState, (void *) ad);
}

void elem_free(void * state) {
    free(state);
}

void elem_process(
        void * state, rage_TransportState const transport_state, rage_Ports const * ports) {
    amp_data const * const data = (amp_data *) state;
    rage_InterpolatedValue const * val = rage_interpolated_view_value(ports->controls[0]);
    uint32_t n_to_change, remaining = data->period_size;
    switch (transport_state) {
        case RAGE_TRANSPORT_STOPPED:
            n_to_change = data->period_size;
            break;
        case RAGE_TRANSPORT_ROLLING:
            n_to_change = (data->period_size < val->valid_for) ?
                data->period_size : val->valid_for;
            break;
    }
    while (1) {
        for (unsigned s = 0; s < n_to_change; s++) {
            for (unsigned c = 0; c < data->n_channels; c++) {
                ports->outputs[c][s] = ports->inputs[c][s] * val->value[0].f;
            }
        }
        rage_interpolated_view_advance(ports->controls[0], n_to_change);
        remaining -= n_to_change;
        if (remaining) {
            val = rage_interpolated_view_value(ports->controls[0]);
            n_to_change = (data->period_size < val->valid_for) ?
                data->period_size : val->valid_for;
        } else {
            break;
        }
    }
}

rage_ElementType const elem_info = {
    .parameters = &init_params,
    .state_new = elem_new,
    .state_free = elem_free,
    .get_ports = elem_describe_ports,
    .free_ports = elem_free_port_description,
    .process = elem_process
};
