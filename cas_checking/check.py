def __build_graph(all_cases):
    deg_in = {}
    deg_out = {}
    for cur_cas in all_cases:
        if cur_cas.result:
            if cur_cas.value_to not in deg_in:
                deg_in[cur_cas.value_to] = 0
            if cur_cas.value_from not in deg_out:
                deg_out[cur_cas.value_from] = 0
            deg_in[cur_cas.value_to] += 1
            deg_out[cur_cas.value_from] += 1
    return deg_in, deg_out


def check(all_cases, init_value):
    start = None
    finish = None
    deg_in, deg_out = __build_graph(all_cases)
    for cur_value in deg_in.keys() | deg_out.keys():
        cur_deg_in = deg_in.get(cur_value, 0)
        cur_deg_out = deg_out.get(cur_value, 0)
        if cur_deg_in == cur_deg_out:
            continue
        elif cur_deg_in == cur_deg_out + 1:
            if finish is not None:
                return False
            else:
                finish = cur_value
        elif cur_deg_in == cur_deg_out - 1:
            if start is not None:
                return False
            else:
                start = cur_value
    return (start is None and finish is None) or \
           (start == init_value and finish is not None and finish != init_value)
