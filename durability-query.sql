CREATE OR REPLACE FUNCTION time_hop(main_table text, t1 integer, t2 integer, tau integer, k integer) RETURNS int[]
AS $$

ans = []
t = t2
flag = 0
while t >= t1:
    t_idx = max(1, t - tau + 1)
    topk_query = 'SELECT id from {} WHERE idx={}'.format(main_table, t_idx)
    topk = plpy.execute(topk_query)
    
    for item in topk[0]['id']:
        if item == t+1 or item == t:
            ans.append(t)
            flag = 1
    if flag == 1:
        t -= 1
        flag = 0
    else:
        max_t = 0
        for item in topk[0]['id']:
            if int(item) > max_t:
                max_t = int(item)
        t = max_t
return ans

$$ LANGUAGE plpython3u;

CREATE OR REPLACE FUNCTION time_sequential(main_table text, t1 integer, t2 integer, tau integer, k integer, params real[], attrs text[]) RETURNS int[]
AS $$

ans = []
t = t2
flag = 1
get_record_sql = 'SELECT score, t, player, game FROM {}'.format(\
                 main_table)
all_records = plpy.execute(get_record_sql)

while t >= t1:
    if flag >= 0:
        topk_query = 'SELECT score, t, player, game FROM {} WHERE t>={} and t<={} ORDER BY score DESC, t limit {}'.format(\
                     main_table, t-tau, t, k)
        raw_topk = plpy.execute(topk_query, k)
        topk = [(d['score'], d['t'], d['player'], d['game']) for d in raw_topk]
        flag = -1
    else:
        new_record = all_records[t-tau]
        new_record = (new_record['score'], new_record['t'], new_record['player'], new_record['game'])
        min_idx = -1
        min_t = topk[0][1]
        min_score = topk[0][0]
        for i in range(k):
            if topk[i][0] < min_score or (topk[i][0] == min_score and topk[i][1] > min_t):
                min_score = topk[i][0]
                min_t = topk[i][1]
                min_idx = i

        if new_record[0] >= min_score:
            topk[min_idx] = new_record

    
    for i in range(k):
        if topk[i][1] == t:
            ans.append(topk[i][1])
            flag = 1
            break
    t -= 1

return ans

$$ LANGUAGE plpython3u;

CREATE OR REPLACE FUNCTION time_sequential_v2(main_table text, index_table text, t1 integer, t2 integer, tau integer, k integer, block_size integer) RETURNS int[]
AS $$
import heapq
ans = []
t = t2
flag = 1
prev_t = t2 - tau - 100000
all_records_sql = 'SELECT t, 0.5*v1+0.5*v2 as score from {} where t>={} and t<={} order by t desc'.format(\
                        main_table, prev_t, t2-tau)
all_records = plpy.execute(all_records_sql)
while t >= t1:
    if prev_t == t - 100000:
        prev_t = t - tau - 100000
        all_records_sql = 'SELECT t, 0.5*v1+0.5*v2 as score from {} where t>={} and t<={} order by t desc'.format(\
                        main_table, prev_t, t-tau)
        all_records = plpy.execute(all_records_sql)
    if flag >= 0:
        topk_query = 'SELECT * from topk(\'{}\', \'{}\', {}, {}, {}, {})'.format(\
                main_table, index_table, t-tau, t, k, block_size)
        topk = plpy.execute(topk_query)
        topk = topk[0]
        score_sql = 'SELECT t, 0.5*v1 + 0.5*v2 as score from {} where '.format(main_table)
        conditions = []
        for item in topk['topk']:
            conditions.append('t={}'.format(item))
        score_sql += ' or '.join(conditions)
        scores = plpy.execute(score_sql)
        heap_scores = []
        for item in scores:
            heap_scores.append((item['score'], item['t']))
        heapq.heapify(heap_scores)
        flag = -1
    else:
        if all_records[(t2-t)%100000]['score'] >= heap_scores[0][0]:
            for idx in range(len(topk['topk'])):
                if topk['topk'][idx] == heap_scores[0][1]:
                    topk['topk'][idx] = t-tau
            heapq.heapreplace(heap_scores, (all_records[(t2-t)%100000]['score'], t-tau))
    
    for item in topk['topk']:
        if int(item) == t+1 or int(item) == t:
            ans.append(t)
            flag = 1
            break
    t -= 1

return ans

$$ LANGUAGE plpython3u;

CREATE OR REPLACE FUNCTION topk(main_table text, index_table text, t1 integer, t2 integer, k integer, block_size integer) RETURNS int[]
AS $$

ans = []
block1 = t1 / block_size
block2 = t2 / block_size
topk_block_sql = 'SELECT idx FROM {} WHERE idx>={} AND idx<={} ORDER BY max_score desc limit {}'.format(\
                    index_table, block1, block2, k)
topk_blocks = plpy.execute(topk_block_sql)

topk_sql = 'SELECT t, 0.5*v1+0.5*v2 as score from {} WHERE '.format(main_table)
conditions = []
for item in topk_blocks:
    block_id = item['idx']
    conditions.append('t>={} AND t<{}'.format(block_id * block_size, (block_id+1) * block_size))
topk_sql += ' OR '.join(conditions)
topk_sql += ' order by score desc limit {}'.format(2*k)
topk_result = plpy.execute(topk_sql)

for item in topk_result:
    if len(ans) >= k:
        break
    if item['t'] >= t1 and item['t'] <= t2:
        ans.append(item['t'])
return ans

$$ LANGUAGE plpython3u;

CREATE OR REPLACE FUNCTION time_hop_v2(main_table text, index_table text, t1 integer, t2 integer, tau integer, k integer, block_size integer) RETURNS int[]
AS $$

ans = []
t = t2
flag = 0
while t >= t1:
    topk_query = 'SELECT * from topk(\'{}\', \'{}\', {}, {}, {}, {})'.format(\
                main_table, index_table, t-tau, t, k, block_size)
    topk = plpy.execute(topk_query)
    
    for item in topk[0]['topk']:
        if item == t+1 or item == t:
            ans.append(t)
            flag = 1

    if flag == 1:
        t -= 1
        flag = 0
    else:
        max_t = 0
        for item in topk[0]['topk']:
            if item > max_t:
                max_t = item
        t = max_t
return ans

$$ LANGUAGE plpython3u;
