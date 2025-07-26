from protocol import run_protocol

# Example data
v_list = ['u1', 'u2', 'u3']
w_t_list = [('u2', 10), ('u3', 20), ('u4', 40)]

sum_result, intersection = run_protocol(v_list, w_t_list)
print("Intersection indexes:", intersection)
print("Intersection sum:", sum_result)
