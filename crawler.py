from flask import Flask, request, jsonify, escape, render_template, make_response, Blueprint
from bs4 import BeautifulSoup
import requests
import json
import urllib.request

app = Flask(__name__)

crawler_api = Blueprint('crawler_api', __name__)


@crawler_api.route('/select_crawler')
def crawl_result():
    args = request.args
    print(args)
    crawler_name = args.get('select_product')

    print(crawler_name)

    totalProducts = []
    #긁어 올 페이지 수
    for pageNumber in range(1, 2):
        pageString = crawl(pageNumber,crawler_name)
        products = parse(pageString)
        totalProducts += products

    #print("total : ",type(totalProducts))
    #select_crawler_result = json.dump(totalProducts)

    print("total : " ,totalProducts)

    products_name = totalProducts[0]['name']
    price = totalProducts[0]['price']
    link = totalProducts[0]['link']
    img = totalProducts[0]['Img']

    print("products : ", products_name)
    print("price : ", price)
    print("link : ", link)

    #lst = [{'a' : '1', 'b' : '2', 'c' : '3'}, {'a' : '1', 'b' : '2', 'c' : '3'}, {'a' : '1', 'b' : '2', 'c' : '3'}]
    #for a in lst:



    return render_template('crawler.html', totalProducts = totalProducts)


def getProductInfo(li):
    # print(li)
    img = li.find("img")  # 태그
    alt = img['alt']  # 속성
    src = img.get('data-original')

    priceReload = li.find("span", {"class": "_price_reload"})
    # span 속성을 class _price_reload 이름

    aTit = li.find("a", {"class": "tit"})
    href = aTit['href']
    # a태그 class tit 이름

    crawler = {'name': alt, "price": priceReload.text, \
               #.text#.replace(",", ""), \
               "link": href, "Img": src}

    print(crawler)
    return crawler


def parse(pageString):
    bsObj = BeautifulSoup(pageString, "html.parser")

    ul = bsObj.find("ul", {"class": "goods_list"})
    lis = ul.findAll("li", {"class": "_itemSection"})

    products = []
    for li in lis[:]:
        try:
            product = getProductInfo(li)
            products.append(product)
        except:
            print("--error--")

    return products



def crawl(pageNumber,result):
    url = "https://search.shopping.naver.com/search/all.nhn?query=" + result + \
          "&pagingIndex={}&cat_id=&frm=NVSHATC".format(pageNumber)
    data = requests.get(url)
    print(data.status_code, url)

    return data.content







# 이미지 저장
"""
    i = 0
    for Img in totalProducts:

    i = i + 1
    img_name = str(i) + ".png"
    Imgdic = Img['Img']
    urllib.request.urlretrieve(Imgdic[:],"./img/" + img_name)"""

# print(totalProducts)
# print(len(totalProducts))

# file = open("./Cardigan.json","w+")
# file.write(json.dumps(totalProducts))



