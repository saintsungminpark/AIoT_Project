from flask import Flask, request, jsonify, escape, render_template, make_response, Blueprint
import requests
import json
import urllib
from flaskext.mysql import MySQL

import sensorDataAPI
from sensorDataAPI import sensor_api
from page_routes import routes_api
from crawler import crawler_api

app = Flask(__name__)

#mysql 설정정보 입력
app.config['MYSQL_DATABASE_HOST'] = 'team05.ckzbwnwwxarf.ap-northeast-2.rds.amazonaws.com'
app.config['MYSQL_DATABASE_USER'] = 'admin'
app.config['MYSQL_DATABASE_PASSWORD'] = 'asdqwe123#team05'
app.config['MYSQL_DATABASE_DB'] = 'babycare_HJ'

#mysql 접속
mysql = MySQL(); mysql.init_app(app)

# # for blueprint
app.register_blueprint(sensor_api)
app.register_blueprint(routes_api)
app.register_blueprint(crawler_api)

#사용자 이름 - 전역 설정
u_name = ""


# index or home page
@app.route('/')
@app.route('/index')
@app.route('/home')
@app.route('/main')
def index():
    title = 'Hello BABY'

    return render_template('index.html', title = title)

# #  test for db read
# @app.route('/gyro/read')
# def read_gyrosensor():
#     title = 'Hello BABY'

#     cursor = mysql.get_db().cursor()

#     cursor.execute("SELECT * FROM babycare_HJ.gyro_sensor")
#     result = cursor.fetchall()

#     return render_template('index.html', title = title)

# login page
@app.route('/login')
def login():
    return render_template('login.html')

# @app.route('/show')
# def show():
#     args = request.args
#     u_id = args.get('u_id')

#     return render_template('display_sensor.html', u_id=u_id)

# check name
@app.route('/name_check', methods=['GET','POST'])
def login_check():
    # if request.method == 'POST':
        if request.headers['Content-Type'] == 'application/json':
            body = request.json

            print(body)
            u_name = body['u_name']
            u_age = body['u_age']
            # u_name = body['u_name']
            # age = body['u_age']
            # print("ajax : ",body)
            print("u_name : ", u_name)
            print("age : ", u_age)

            u_info_list = []

            cursor = mysql.get_db().cursor()

            cursor.execute("select u_id,u_age,u_name \
                from users \
                where u_name = %s ", (u_name) )
            check = cursor.fetchall()

            print(check)

            if len(check) > 0:
                for result in check:
                    u_info_list.append(result)

                u_id = u_info_list[0][0]
                age = u_info_list[0][1]
                u_name = u_info_list[0][2]
                print("id : ",u_id)
                print("age : ",age)
                print("name : ",  u_name)
                return_data={
                    "u_id" : u_id,
                    "u_age" : age,
                    "u_name": u_name
                }
                return jsonify(return_data)
                #return render_template("FlaksToHtml_JSON.html")
            else:
                u_id = ''
                u_age = ' '
                u_name = ' '
                return_data={
                    "u_id" : u_id,
                    "u_age" : u_age,
                    "u_name": u_name
                }
                print(return_data)
                # ajax
                return return_data


# logout 
@app.route('/logout')
def logout():
    resp = make_response(render_template('index.html'))
    resp.set_cookie('userID', expires=0)

    return resp

#baby name and age check
@app.route('/register')
def register():
    """print(name)
    print(age)"""
    title= 'register'
    args = request.args
    name = args.get('regi_name'); age = args.get('regi_age')
    kakao_token = args.get('kakao_token')
    print("token : ",kakao_token)
    
    cursor = mysql.get_db().cursor()
    cursor.execute("insert into users (u_age, u_name,refresh_token_kakao) \
        values (%s, %s, %s)", (age, name,kakao_token))

    mysql.get_db().commit()
    # db 입력 완료 될 시.

    send = sensorDataAPI.getAccessToken_refresh(kakao_token)
    kakao = send['access_token']
    sensorDataAPI.sendText(kakao, name, "확인")

    return render_template("join.html", title =title,name=name, age=age)
    # return render_template("register.html", title =title,name=name, age=age)


"""@app.route('/oauth')
def oauth():
    code = str(request.args.get('code'))
    resToken = getAccessToken("8dd5d0eab9cb12d172d1e4eaf5e72b7c",str(code))  #XXXXXXXXX 자리에 RESET API KEY값을 사용
    refresh_token = resToken['refresh_token']

    print("resToken : ",resToken)

    result =  getAccessToken_refresh(refresh_token)
    #print("result : ", result)

    #print ("######  : " + result['access_token'])
    #print ("Send : ",sendText(result['access_token'],1,1))

    # return 'code=' + str(code) + '<br/>response for token=' + str(resToken)
    # redirect to home
    # '/'

    # return 'code=' + str(code) + '<br/>response for token=' + str(resToken + '<br/>response for refresh token=' + str(result)
    return render_template("join.html", refresh_token = refresh_token)


def getAccessToken(clientId, code) :  # 세션 코드값 code 를 이용해서 ACESS TOKEN과 REFRESH TOKEN을 발급 받음
    url = "https://kauth.kakao.com/oauth/token"
    payload = "grant_type=authorization_code"
    payload += "&client_id=" + clientId
    payload += "&redirect_url=http%3A%2F%2Flocalhost%3A5000%2Foauth&code=" + code
    headers = {
        'Content-Type' : "application/x-www-form-urlencoded",
        'Cache-Control' : "no-cache",
    }
    reponse = requests.request("POST",url,data=payload, headers=headers)
    access_token = json.loads(((reponse.text).encode('utf-8')))
    return access_token

def getAccessToken_refresh(refreshToken) :  # 메세지 받을 사람의 REFRESH TOKEN 이용
    url = "https://kauth.kakao.com/oauth/token"
    payload = "grant_type=refresh_token&client_id=8dd5d0eab9cb12d172d1e4eaf5e72b7c&refresh_token=" + refreshToken
    headers = {
        'Content-Type' : "application/x-www-form-urlencoded",
        'Cache-Control' : "no-cache",
    }
    reponse = requests.request("POST",url,data=payload, headers=headers)
    access_token = json.loads(((reponse.text).encode('utf-8')))
    return access_token


def sendText(accessToken,text,title) :
    url = 'https://kapi.kakao.com/v2/api/talk/memo/default/send'
    data_json = request.json

    payloadDict = dict({
            "object_type" : "text",
            "text" : text,
            "link" : {
                "web_url" : "http://www.naver.com",
                "mobile_web_url" : "http://m.naver.com"
             },
        "buttons": [
            {
                "title": title,
                "link": {
                    "web_url": "http://www.naver.com",
                    "mobile_web_url": "http://m.naver.com",
                    "android_execution_params": "/=true",
                    "ios_execution_params": "/=true"
                }
            }
        ]
            })

    payload = 'template_object=' + str(json.dumps(payloadDict))
    print (payload)
    headers = {
        'Content-Type' : "application/x-www-form-urlencoded",
        'Cache-Control' : "no-cache",
        'Authorization' : "Bearer " + accessToken,
    }
    reponse = requests.request("POST",url,data=payload, headers=headers)
    access_token = json.loads(((reponse.text).encode('utf-8')))
    return access_token"""


#일단 무시
@app.route('/username')
def userName() :
    args = request.args
    u_name = args.get('u_name')
    print(u_name)

    return render_template("heart.html")

@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html')

if __name__ =='__main__':
    app.run(host="0.0.0.0" ,debug=True)