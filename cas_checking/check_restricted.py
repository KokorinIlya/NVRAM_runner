from utils import flatten


def __build_graph(all_cases, init_value):
    graph = {}
    total_edges = 0
    value_to_seen = set()
    for cur_cas in all_cases:
        if cur_cas.value_from == cur_cas.value_to:
            raise Exception('value_from == value_to')
        if cur_cas.value_to == init_value:
            raise Exception('value_to == init_value')
        if cur_cas.value_to in value_to_seen:
            raise Exception('Duplicated value_to')
        value_to_seen.add(cur_cas.value_to)

        if cur_cas.result:
            total_edges += 1
            if cur_cas.value_from not in graph:
                graph[cur_cas.value_from] = []
            graph[cur_cas.value_from].append(cur_cas.value_to)
    return graph, total_edges


def __traverse_graph(graph, init_value):
    traversed_edges = 0
    index_by_edge = {}
    cur_value = init_value
    while cur_value in graph and len(graph[cur_value]) > 0:
        if len(graph[cur_value]) > 1:
            return False
        next_value = graph[cur_value][0]
        index_by_edge[(cur_value, next_value)] = traversed_edges
        cur_value = next_value
        traversed_edges += 1
    return index_by_edge, traversed_edges


def check(cases_by_thread, init_value):
    all_cases = flatten(cases_by_thread)
    graph, total_edges = __build_graph(all_cases, init_value)

    if init_value not in graph:
        if total_edges == 0:
            for cur_cas in all_cases:
                if cur_cas.value_from == init_value:
                    return False
            return True
        else:
            return False

    index_by_edge, traversed_edges = __traverse_graph(graph, init_value)
    if traversed_edges != total_edges:
        return False
    for cur_thread_cases in cases_by_thread:
        only_true_cases = [cur_cas for cur_cas in cur_thread_cases if cur_cas.result]
        for i in range(len(only_true_cases) - 1):
            cur_cas = only_true_cases[i]
            next_cas = only_true_cases[i + 1]
            cur_cas_index = index_by_edge[(cur_cas.value_from, cur_cas.value_to)]
            next_cas_index = index_by_edge[(next_cas.value_from, next_cas.value_to)]
            if cur_cas_index >= next_cas_index:
                return False
    return True
