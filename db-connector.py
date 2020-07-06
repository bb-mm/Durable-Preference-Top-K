import psycopg2
import heapq
import time
import random
from collections import defaultdict
import numpy as np

def get_top_k(conn, table, t1, t2, params, attrs, k):
    # get cursor
    cur = conn.cursor()

    sql = 'SELECT {} AS score, t, player, game FROM {} WHERE t>={} and t<={} ORDER BY score DESC, t'.format(\
                '+'.join([str(a)+'*'+b for a,b in zip(params, attrs)]), table, t1, t2)
    cur.execute(sql)

    rows = cur.fetchmany(k)

    return rows

def get_record(conn, table, t, params, attrs):
    cur = conn.cursor()
    sql = 'SELECT {} AS score, t, player, game FROM {} WHERE t={}'.format(\
            '+'.join([str(a)+'*'+b for a,b in zip(params, attrs)]), table, t)
    cur.execute(sql)
    record = cur.fetchone()
    # print(type(record))
    # print(record)
    return record

def answer_check(a, b):
    a.sort()
    b.sort()
    print(len(a))
    print(len(b))
    # for item in zip(a,b):
    #     print(item)
    return a == b

def durable_check(topk, ans, t):
    for i in range(len(topk)):
        if topk[i][1] == t:
            ans.append(topk[i])
            return i
    return -1

def time_sequential_topk(conn, table, t1, t2, params, attrs, tau, k):
    results = []
    topk_queries = 0
    # topk record w.r.t ranking function at the current time window
    topk = []
    t = t2
    restart = 1
    while t >= t1:
        if restart >= 0:
            topk = get_top_k(conn, table, t-tau, t, params, attrs, k)
            topk_queries += 1
            # transfer list into a heap
            heapq.heapify(topk)
        else:
            new_record = get_record(conn, table, t-tau, params, attrs)
            if new_record[0] >= topk[0][0]:
                heapq.heapreplace(topk, new_record)
        
        restart = durable_check(topk, results, t)
        t -= 1
    return results, topk_queries

def time_sequential_k_band(conn, table, t1, t2, params, attrs, tau, k):
    results = []
    return results

def time_hop(conn, table, t1, t2, params, attrs, tau, k):
    results = []
    topk = []
    topk_queries = 0
    t = t2
    found = 1

    while t >= t1:
        topk = get_top_k(conn, table, t-tau, t, params, attrs, k)
        topk_queries += 1
        found = durable_check(topk, results, t)
        if found >= 0:
            t -= 1
        # time hop: directly jump to the latest timestamp in the current topk set
        else:
            t = max([item[1] for item in topk])

    return results, topk_queries

def score_hop(conn, table, t1, t2, params, attrs, tau, k):
    results = []
    return results


def query_efficiency_as_tau(conn, trails, k):
    cur = conn.cursor()

    taus = [100000, 200000, 300000, 400000, 500000]
    t1 = 510000
    t2 = 980000
    hop_times = defaultdict(list)
    sequential_times = defaultdict(list)
    
    for tau in taus:
        print(tau, t1, t2, k)
        for i in range(trails):
            hop_sql = 'select * from time_hop(\'nba_pts_ast_moving_{}k_top_10\', {}, {}, {}, {})'.format(int(tau/1000), t1, t2, tau, k)
            sequential_sql = 'select * from time_sequential_v2(\'nba_pts_ast_moving_{}k_top_10\', {}, {}, {}, {})'.format(int(tau/1000), t1, t2, tau, k)

            if tau == 100000 and i == 0:
                print(hop_sql)
                print(sequential_sql)

            start = time.time()
            cur.execute(hop_sql)
            hop_result = cur.fetchall()
            end = time.time()
            hop_times[tau].append((end-start)*1000)

            start = time.time()
            cur.execute(sequential_sql)
            sequential_result = cur.fetchall()
            end = time.time()
            sequential_times[tau].append((end-start)*1000)
    
    for key in hop_times:
        print('tau {}:{}/{}'.format(key, np.mean(hop_times[key]), np.std(hop_times[key])))
        print('tau {}:{}/{}'.format(key, np.mean(sequential_times[key]), \
                                        np.std(sequential_times[key])))

def query_efficiency_as_L(conn, trails, k):
    cur = conn.cursor()

    Ls = [100000, 200000, 300000, 400000, 500000]
    t2 = 980000
    tau = 500000
    hop_times = defaultdict(list)
    sequential_times = defaultdict(list)
    
    for L in Ls:
        t1 = max(500100, t2 - L)
        print(L, t1, t2, k, tau)
        for i in range(trails):
            hop_sql = 'select * from time_hop(\'nba_pts_ast_moving_500k_top_10\', {}, {}, {}, {})'.format(t1, t2, tau, k)
            sequential_sql = 'select * from time_sequential_v2(\'nba_pts_ast_moving_500k_top_10\', {}, {}, {}, {})'.format(t1, t2, tau, k)
            
            if L == 500000 and i == 0:
                print(hop_sql)
                print(sequential_sql)
            
            start = time.time()
            cur.execute(hop_sql)
            hop_result = cur.fetchall()
            end = time.time()
            hop_times[L].append((end-start)*1000)

            start = time.time()
            cur.execute(sequential_sql)
            sequential_result = cur.fetchall()
            end = time.time()
            sequential_times[L].append((end-start)*1000)
    
    for key in hop_times:
        print('L {}:{}/{}'.format(key, np.mean(hop_times[key]), np.std(hop_times[key])))
        print('L {}:{}/{}'.format(key, np.mean(sequential_times[key]), \
                                        np.std(sequential_times[key])))

def query_efficiency_on_syn(conn, trails, k):
    cur = conn.cursor()
    hop_uni_times = []
    hop_anti_times = []

    t1 = 240000000
    t2 = 490000000
    tau = 100000000

    print(t1, t2, tau, k)

    for i in range(trails):
        hop_uni_sql = 'select * from time_hop(\'syn_uni_moving_100m_top_10\', {}, {}, {}, {})'.format(t1, t2, tau, k)
        hop_anti_sql = 'select * from time_hop(\'syn_anti_moving_100m_top_10\', {}, {}, {}, {})'.format(t1, t2, tau, k)

        start = time.time()
        cur.execute(hop_uni_sql)
        uni_result = cur.fetchall()
        end = time.time()
        if i >= 0:
            hop_uni_times.append((end-start)*1000)

        start = time.time()
        cur.execute(hop_anti_sql)
        anti_result = cur.fetchall()
        end = time.time()
        if i >= 0:
            hop_anti_times.append((end-start)*1000)
    
    print('uniform:{}/{}'.format(np.mean(hop_uni_times), np.std(hop_uni_times)))
    print('anti:{}/{}'.format(np.mean(hop_anti_times), np.std(hop_anti_times)))

if __name__ == '__main__':
    ''' Connect to the PostgreSQL database server '''
    conn = psycopg2.connect('dbname=temporal_db user=jygao')
    if conn is not None:
        print('connected')
    
    
    # query_efficiency_as_tau(conn, 10, 10)
    
    # query_efficiency_as_L(conn, 10, 10)

    query_efficiency_on_syn(conn, 10, 100)
    
    # t1 = 400010
    # t2 = 980000
    # params = [0.5, 0.5]
    # attrs = ['pts', 'ast']
    # k = 5
    # tau = 400000
    # table = 'nba_pts_ast'
    
    # start = time.time()
    # ans_seq_time, topk_cnt = time_sequential_topk(conn, table, t1, t2, params, attrs, tau, k)
    # end = time.time()
    # print(topk_cnt, end-start)

    # start = time.time()
    # ans_time_hop, topk_cnt = time_hop(conn, table, t1, t2, params, attrs, tau, k)
    # end = time.time()
    # # for item in ans_time_hop:
    # #     print(item[1])
    # print(topk_cnt, end-start)

    # print(answer_check(ans_seq_time, ans_time_hop))
    