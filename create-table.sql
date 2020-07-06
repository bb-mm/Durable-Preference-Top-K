-- create table

--nba data
--create table nba_pts_ast (player varchar(64), pts real, ast real, game varchar(64));
-- create table nba_pts_trb (player varchar(64), pts real, trb real, game varchar(64));
-- create table nba_ast_trb (player varchar(64), ast real, trb real, game varchar(64));
-- create table nba_pts_ast_trb (player varchar(64), pts real, ast real, trb real, game varchar(64));
-- create table nba_pts_ast_trb_stl_blk (player varchar(64), pts real, ast real, trb real, stl real, blk real, game varchar(64));

--syn data
-- create table syn_uni (dim int, v1 real, v2 real, t int primary key);
-- create table syn_circle (dim int, v1 real, v2 real, t int primary key);
-- create table syn_circle_v2 (dim int, v1 real, v2 real, t int primary key);
-- create table syn_anti (dim int, v1 real, v2 real, t int primary key);

-- moving topk table
-- create table nba_pts_ast_moving_100k_top_10 (idx int primary key, id int[]);
-- create table nba_pts_ast_moving_200k_top_10 (idx int primary key, id int[]);
-- create table nba_pts_ast_moving_300k_top_10 (idx int primary key, id int[]);
-- create table nba_pts_ast_moving_400k_top_10 (idx int primary key, id int[]);
-- create table nba_pts_ast_moving_500k_top_10 (idx int primary key, id int[]);

-- create table syn_anti_moving_100m_top_10 (idx int primary key, id int[]);
create table syn_uni_moving_100m_top_10 (idx int primary key, id int[]);

--load data
-- copy nba_pts_trb(player, pts, trb, game) from '/home/jygao/workspace/durablepreferencetopk/data/nba_full_PTS_TRB.csv' delimiter ' ' CSV;
-- copy nba_ast_trb(player, ast, trb, game) from '/home/jygao/workspace/durablepreferencetopk/data/nba_full_AST_TRB.csv' delimiter ' ' CSV;
-- copy nba_pts_ast_trb(player, pts, ast, trb, game) from '/home/jygao/workspace/durablepreferencetopk/data/nba_full_PTS_AST_TRB.csv' delimiter ' ' CSV;
-- copy nba_pts_ast_trb_stl_blk(player, pts, ast, trb, stl, blk, game) from '/home/jygao/workspace/durablepreferencetopk/data/nba_full_PTS_AST_TRB_STL_BLK.dat' delimiter ' ' CSV;

-- copy syn_uni(dim, v1, v2, t) from '/home/jygao/workspace/durablepreferencetopk/data/syn_50M.csv' delimiter ' ' CSV;
-- copy syn_circle(dim, v1, v2, t) from '/home/jygao/workspace/durablepreferencetopk/data/syn_50M_CIRCLE.csv' delimiter ' ' CSV;
-- copy syn_circle_v2(dim, v1, v2, t) from '/home/jygao/workspace/durablepreferencetopk/data/syn_50M_CIRCLE_v2.csv' delimiter ' ' CSV;

-- copy syn_anti(dim, v1, v2, t) from '/home/jygao/workspace/durablepreferencetopk/data/syn_500M_CIRCLE.csv' delimiter ' ' CSV;

-- copy syn_uni(dim, v1, v2, t) from '/home/jygao/workspace/durablepreferencetopk/data/syn_500M_IND.csv' delimiter ' ' CSV;

-- copy nba_pts_ast_moving_100k_top_10 (idx, id) from '/home/jygao/workspace/durablepreferencetopk/data/nba_full_PTS_AST_moving_100000_top_10.csv' delimiter ' ' CSV;
-- copy nba_pts_ast_moving_200k_top_10 (idx, id) from '/home/jygao/workspace/durablepreferencetopk/data/nba_full_PTS_AST_moving_200000_top_10.csv' delimiter ' ' CSV;
-- copy nba_pts_ast_moving_300k_top_10 (idx, id) from '/home/jygao/workspace/durablepreferencetopk/data/nba_full_PTS_AST_moving_300000_top_10.csv' delimiter ' ' CSV;
-- copy nba_pts_ast_moving_400k_top_10 (idx, id) from '/home/jygao/workspace/durablepreferencetopk/data/nba_full_PTS_AST_moving_400000_top_10.csv' delimiter ' ' CSV;
-- copy nba_pts_ast_moving_500k_top_10 (idx, id) from '/home/jygao/workspace/durablepreferencetopk/data/nba_full_PTS_AST_moving_500000_top_10.csv' delimiter ' ' CSV;
-- copy syn_anti_moving_100m_top_10 (idx, id) from '/home/jygao/workspace/durablepreferencetopk/data/syn_500M_CIRCLE_moving_100M_top_10.csv' delimiter ' ' CSV;
copy syn_uni_moving_100m_top_10 (idx, id) from '/home/jygao/workspace/durablepreferencetopk/data/syn_500M_IND_moving_100M_top_10.csv' delimiter ' ' CSV;


-- query
-- select array_length(time_hop, 1) from time_hop('nba_pts_ast_moving_100k_top_10', 500000, 700000, 100000, 10);
-- select array_length(time_sequential_v2, 1) from time_sequential_v2('nba_pts_ast_moving_100k_top_10', 500000, 700000, 100000, 10);
-- select array_length(time_hop, 1) from time_hop('syn_uni_moving_100m_top_10', 200000000, 450000000, 100000000, 10);
-- select array_length(time_sequential_v2, 1) from time_sequential_v2('syn_uni_moving_100m_top_10', 200000000, 450000000, 100000000, 10);