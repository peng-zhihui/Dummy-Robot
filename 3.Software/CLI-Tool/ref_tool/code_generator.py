
import jinja2
import os
import json

def get_flat_endpoint_list(json, prefix, id_offset):
    flat_list = []
    for item in json:
        item = item.copy()
        if 'id' in item:
            item['id'] -= id_offset
        if 'type' in item:
            if item['type'] in {'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64'}:
                item['type'] += '_t'
                is_property = True
            elif item['type'] in {'bool', 'float'}:
                is_property = True
            elif item['type'] in {'function'}:
                if len(item.get('arguments', [])) == 0 and len(item.get('inputs', [])) == 0 and len(item.get('outputs', [])) == 0:
                    item['type'] = 'void'
                    is_property = True
                else:
                    is_property = False
            else:
                is_property = False
            if is_property:
                item['name'] = prefix + item['name']
                flat_list.append(item)
        if 'members' in item:
            flat_list = flat_list + get_flat_endpoint_list(item['members'], prefix + item['name'] + '.', id_offset)
    return flat_list

def generate_code(odrv, template_file, output_file):
    json_data = odrv._json_data
    json_crc = odrv._json_crc

    axis0_json = [item for item in json_data if item['name'].startswith("axis0")][0]
    axis1_json = [item for item in json_data if item['name'].startswith("axis1")][0]
    json_data = [item for item in json_data if not item['name'].startswith("axis")]
    endpoints = get_flat_endpoint_list(json_data, '', 0)
    per_axis_offset = axis1_json['members'][0]['id'] - axis0_json['members'][0]['id']
    axis_endpoints = get_flat_endpoint_list(axis0_json['members'], 'axis.', 0)
    axis_endpoints_copy = get_flat_endpoint_list(axis1_json['members'], 'axis.', per_axis_offset)
    if axis_endpoints != axis_endpoints_copy:
        raise Exception("axis0 and axis1 don't look exactly equal")

    env = jinja2.Environment(
        #loader = jinja2.FileSystemLoader("/Data/Projects/")
        #trim_blocks=True,
        #lstrip_blocks=True
    )

    # Expose helper functions to jinja template code
    #env.filters["delimit"] = camel_case_to_words

    #import ipdb; ipdb.set_trace()

    # Load and render template
    template = env.from_string(template_file.read())
    output = template.render(
        json_crc=json_crc,
        endpoints=endpoints,
        per_axis_offset=per_axis_offset,
        axis_endpoints=axis_endpoints,
        output_name=os.path.basename(output_file.name)
    )

    # Output
    output_file.write(output)
