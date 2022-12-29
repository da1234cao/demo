import requests 
from lxml import etree
import time
import random
import wordcloud
import matplotlib.pyplot as plt
import numpy as np

param = {
    'user_name': 'sinat_38816924', # 用户名
    'blog_size': 77 # 需要分析的最新发布的博客数(eg. 今年的博客数量)
}

data = {
    'blog': [], # 博客的名称，链接，发布时间，标签，点赞数，评论数
    'label': {}, # 标签，存储每个标签出现过的次数
    'digg_count': 0, # 点赞数
    'comment_count': 0, # 评论数
}

def get_data():
    page_size = 20 # csdn默认的每次加载的博客数,不要修改这个值
    cycle = param['blog_size'] // page_size
    if(param['blog_size'] % page_size != 0):
        cycle = cycle + 1
    
    # 获取总体数据
    for i in range(cycle+1):
        if(i == 0):
            continue # i=0不需要
        if(i == cycle):
            page_size = param['blog_size'] - page_size * (i - 1) # 最后一次可能不足20

        url = 'https://blog.csdn.net/community/home-api/v1/get-business-list?&businessType=lately&noMore=false'
        headers = {
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36",
        }
        payload = {'page': i, 'size': page_size, 'username': param['user_name']}
        resp = requests.get(url, params=payload, timeout=30, headers=headers)
        resp.encoding = 'utf-8'
        if resp.status_code == 200:
            content_list = resp.json()['data']['list']
            for blog in content_list:
                blog_dict = {}
                blog_dict['name'] = blog['title']
                blog_dict['url'] = blog['url']
                blog_dict['create_time'] = blog['createTime']
                blog_dict['digg_count'] = blog['diggCount']
                blog_dict['comment_count'] = blog['commentCount']
                data['blog'].append(blog_dict)

    # 获取每篇博客的标签
    for i,blog in enumerate(data['blog']):
        url = blog['url']
        resp = requests.get(url, timeout=30, headers=headers)
        # with open("tmp.txt", "w", encoding='utf-8') as o:
        #     o.write(resp.text)
        etree_html = etree.HTML(resp.text)
        content = etree_html.xpath('//*[@id="mainBox"]/main/div[1]/div[1]/div/div[2]/div[2]/div/a/text()')
        content.pop(0) # 第一个元素是分类专栏，非标签，丢弃
        data['blog'][i]['label'] = content
        print('获取到第%d篇博客信息' % (i+1))
        time.sleep(random.randint(0,1)) # 休息下，放慢爬取的频率 
        # break

def analyze_data():
    for blog in data['blog']:
        data['digg_count'] = data['digg_count'] + blog['digg_count']
        data['comment_count'] = data['comment_count'] + blog['comment_count']
        labels = blog['label']
        for label in labels:
            if(label in data['label']):
                data['label'][label] = data['label'][label] + 1
            else:
                data['label'][label] = 1
        # break

def print_data():
    print('过去一年，总共发布%d篇博客' % len(data['blog']))
    print('过去一年，收到了%d个点赞' % data['digg_count'])
    print('过去一年，收到了%d个评论' % data['comment_count'])

def draw_data():
    # 将数据绘制成图表

    # 将所有标签绘制成词云
    label_str = ''
    for label in data['label']:
        for freq in range(data['label'][label]):
            label_str = label_str + label + ' '
    # 微软雅黑字体, collocations=false,不需要搭配,因为每次词已经是标签了
    w = wordcloud.WordCloud(font_path='msyh.ttc', collocations=False)
    w.generate(label_str)
    w.to_file('lable_world_clould.png')
    print('绘制词云图片')

    # 按照时间绘制博客数量的柱状图
    month_cnt = [0]*12 # 记录每个月的博客数量
    for blog in data['blog']:
        timestamp = blog['create_time'] // 1000 # 去除末尾的3个零
        time_local = time.localtime(timestamp) # 将时间戳转换成时间结构
        time_month = time_local.tm_mon # 博客创建的月份
        month_cnt[time_month - 1] = month_cnt[time_month - 1] + 1
    # print(month_cnt)
    # 这两行代码解决 plt 中文显示的问题
    plt.rcParams['font.sans-serif'] = ['SimHei']
    plt.rcParams['axes.unicode_minus'] = False
    X = range(1, 13) # x轴
    plt.bar(X, month_cnt)
    plt.title('每月发布的博客数量')
    plt.savefig("month_cnt.png")
    print('绘制博客数量的柱状图')

       
if __name__ == "__main__":
    get_data()
    analyze_data()
    print_data()
    draw_data()
