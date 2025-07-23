from party1 import Party1
from party2 import Party2

def run_protocol(v_list, w_t_list):
    P1 = Party1(v_list)
    P2 = Party2(w_t_list)

    X = P1.round1()
    pk = P2.get_public_key()
    Z, data = P2.round2(X)

    P1.set_Z(Z)
    ct_sum = P1.round3(data, pk)

    final_sum = P2.decrypt_sum(ct_sum)
    return final_sum, P1.intersection


