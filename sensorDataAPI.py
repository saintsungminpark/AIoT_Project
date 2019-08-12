from flask import Flask, request, jsonify, escape, render_template, make_response, Blueprint
import requests
import json
import urllib
from flaskext.mysql import MySQL

app = Flask(__name__)

#mysql 설정정보 입력
app.config['MYSQL_DATABASE_HOST'] = 'team05.ckzbwnwwxarf.ap-northeast-2.rds.amazonaws.com'
app.config['MYSQL_DATABASE_USER'] = 'admin'
app.config['MYSQL_DATABASE_PASSWORD'] = 'asdqwe123#team05'
app.config['MYSQL_DATABASE_DB'] = 'babycare_HJ'

#mysql 접속
mysql = MySQL(); mysql.init_app(app)

sensor_api = Blueprint('sensor_api', __name__)
# sensor_api = Blueprint('sensor_api', __name__, url_prefix="/api_of_db")


# sensor insert, read(get), update, delete #

# insert start
####zyro sensor data NodeMCU ==> Amazon DB
@sensor_api.route('/sensor/gyro/insert', methods=['POST'])
def insert_gyro():
    req_body = request.json
    
    #req_msg : { "x_gyro" : [100,100,...]
    #            "y_gyro" : [100,100,...]
    #            "z_gyro" : [100,100,...]
    #            "x_gyro_max" : 100,
    #            "y_gyro_max" : 100,  
    #            "z_gyro_max" : 100,  
    #            "u_id" : 1
    #           }
    
    x = req_body['x_gyro'];y = req_body['y_gyro'];z = req_body['z_gyro']
   

    ##서버의 action위해서 x_max, y_max, z_max도 같이 보냄! 코딩 필요..ㅠ
    x_max = req_body['x_gyro_max'];y_max = req_body['y_gyro_max'];z_max = req_body['z_gyro_max']
    u_id = req_body['u_id']

    # x , y, z가 배열이라면.
    x_str = ','.join(map(str, x)); y_str = ','.join(map(str, y)); z_str = ','.join(map(str, z))
    
    cursor = mysql.get_db().cursor()
    sql = "insert into gyro_sensor (x, y, z, u_id) values (%s,%s,%s,%s)"
    cursor.execute( sql, (x_str, y_str, z_str, u_id))
    
    mysql.get_db().commit()

    # # gyro 이상 상태 감지
    # threshhold_angX = 5
    # if x_max > threshhold_angX:
    #     sendmesg = []
    #     sql = "select refresh_token_kakao from users where u_id = (%s)"
    #     cursor.execute(sql, u_id)

    #     check = cursor.fetchone()
    #     sendmesg.append(check)
    #     token = sendmesg[0][0]
    #     print("send : ",token)
    #     print("#########################")
    #     print(getAccessToken_refresh(token))
    #     send = getAccessToken_refresh(token)
    #     kakao = send['access_token']
    #     print("kakao : ", kakao)
    #     sendText(kakao,"아기가 엎드림!","아기에게 가서 다시 돌려주세요.")

    #     return jsonify({"message": "success"})



    return jsonify( {"message" : "success"} )

####heart rate sensor data NodeMCU ==> Amazon DB
@sensor_api.route("/sensor/heart/insert", methods=["POST"])
def insert_heart() :
    req_body = request.json

    #req_msg : { "bpm" : 100,
    #            "u_id" : 1
    #           }

    bpm = req_body["bpm"]
    u_id = req_body["u_id"]

    cursor = mysql.get_db().cursor()
    sql = "insert into heart_sensor (bpm, u_id) values (%s, %s)"
    cursor.execute(sql, (bpm, u_id))

    mysql.get_db().commit()
    
    # heart 이상 상태 감지
    min_threshhold_heart = 60; max_threshhold_heart = 100
    if ( bpm < min_threshhold_heart or bpm > max_threshhold_heart ):
        sendmesg = []
        sql = "select refresh_token_kakao from users where u_id = (%s)"
        cursor.execute(sql, u_id)

        check = cursor.fetchone()
        sendmesg.append(check)
        token = sendmesg[0][0]
        print("send : ",token)
        print("#########################")
        print(getAccessToken_refresh(token))
        send = getAccessToken_refresh(token)
        kakao = send['access_token']
        print("kakao : ", kakao)
        if (bpm < min_threshhold_heart):
            sendText(kakao,"심장박동 느려짐","무슨일인지 확인 바람.")
        else:
            sendText(kakao,"심장박동 빨리짐","무슨일인지 확인 바람.")
        

        return jsonify({"message": "success"})

    return ({"message": "success"})

####sounde_sonser insert data NodeMCU ==> Amazon DB
@sensor_api.route('/sensor/sound/insert', methods=['POST'])
def insert_sound():
    req_body = request.json

    #req_msg : { "db" : 100,
    #            "fq" : 100,
    #            "u_id" : 1
    #           }

    dB = req_body['db']
    fq = req_body['fq']
    u_id = req_body["u_id"]

    cursor = mysql.get_db().cursor()
    sql = "insert into sound_sensor (sound_db,sound_fq,u_id) values (%s,%s,%s)"
    cursor.execute(sql, (dB,fq,u_id))

    mysql.get_db().commit()

    # sound 이상 상태 감지
    threshhold_dB = 130
    if dB > threshhold_dB:
        sendmesg = []
        sql = "select refresh_token_kakao from users where u_id = (%s)"
        cursor.execute(sql, u_id)

        check = cursor.fetchone()
        sendmesg.append(check)
        token = sendmesg[0][0]
        print("send : ",token)
        print("#########################")
        print(getAccessToken_refresh(token))
        send = getAccessToken_refresh(token)
        kakao = send['access_token']
        print("kakao : ", kakao)
        sendText(kakao,"아기가 우는중","왜 우는 지 확인 바람")

        return jsonify({"message": "success"})

    return jsonify({"message": "success"})

####tempature_sensor insert data NodeMCU ==> Amazon DB
@sensor_api.route('/sensor/temp/insert', methods=['POST'])
def insert_temp_hum():
    req_body = request.json

    #req_msg : { "temp" : 100,
    #            "hum" : 100,
    #            "u_id" : 1
    #           }

    temp = req_body['temp']
    hum = req_body['hum']
    u_id = req_body["u_id"]
    print("temp : ",temp)

    cursor = mysql.get_db().cursor()
    sql = "insert into temp_sensor (temp,hum,u_id) values (%s,%s,%s)"
    cursor.execute(sql, (temp,hum,u_id))

    mysql.get_db().commit()

    # temp and hum 이상 상태 감지
    threshhold_temp = 38.5
    if temp > threshhold:
        sendmesg = []
        sql = "select refresh_token_kakao from users where u_id = (%s)"
        cursor.execute(sql, u_id)

        check = cursor.fetchone()
        sendmesg.append(check)
        token = sendmesg[0][0]
        print("send : ",token)
        print("#########################")
        print(getAccessToken_refresh(token))
        send = getAccessToken_refresh(token)
        kakao = send['access_token']
        print("kakao : ", kakao)
        sendText(kakao,"온도 이상","온도 체크")

        return jsonify({"message": "success"})

    return jsonify({"message": "success"})
# insert end

# read(get or select) start
####heart_rate_senser select data Amazon DB ==> Web
@sensor_api.route("/sensor/heart/select/<u_id>")
def select_heart(u_id):

    cursor = mysql.get_db().cursor()
    print("u_id : " , u_id)
    sql = "select * \
    from babycare_HJ.heart_sensor where u_id = (%s)\
    order by datetime DESC limit 10"
    cursor.execute(sql, u_id)

    row_headers = [x[0] for x in cursor.description]
    results = cursor.fetchall()
    print("Heart results : " ,results)

    json_data = []
    for result in results:
        json_data.append(dict(zip(row_headers, result)))

    return jsonify({"values": json_data})

####temperature_senser select data Amazon DB ==> Web
@sensor_api.route("/sensor/temp/select/<u_id>")
def select_temp(u_id) :
    cursor = mysql.get_db().cursor()

    sql = "select * \
    from babycare_HJ.temp_sensor \
    where u_id = (%s)\
    order by datetime DESC limit 10"
    cursor.execute(sql, u_id)

    row_headers = [x[0] for x in cursor.description]
    results = cursor.fetchall()
    # print(results)

    json_data = []
    for result in results:
        json_data.append(dict(zip(row_headers, result)))

    return jsonify({"values": json_data})

@sensor_api.route("/sensor/sound/select/<u_id>")
def select_sound(u_id) :

    cursor = mysql.get_db().cursor()

    sql = "select * \
    from babycare_HJ.sound_sensor \
    where u_id = (%s)\
    order by datetime DESC limit 10"

    cursor.execute(sql, u_id)

    row_headers = [x[0] for x in cursor.description]
    results = cursor.fetchall()
    print(results)

    json_data = []
    for result in results:
        json_data.append(dict(zip(row_headers, result)))

    return jsonify({"values": json_data})

@sensor_api.route("/sensor/gyro/select/<u_id>")
def select_gyro(u_id) :

    cursor = mysql.get_db().cursor()

    sql = "select * \
    from babycare_HJ.gyro_sensor \
    where u_id = (%s)\
    order by datetime DESC limit 10"

    cursor.execute(sql, u_id)

    row_headers = [x[0] for x in cursor.description]
    results = cursor.fetchall()
    print(results)

    json_data = []
    for result in results:
        json_data.append(dict(zip(row_headers, result)))

    return jsonify({"values": json_data})

@sensor_api.route('/oauth')
def oauth():
    code = str(request.args.get('code'))
    resToken = getAccessToken("8dd5d0eab9cb12d172d1e4eaf5e72b7c",str(code))  #XXXXXXXXX 자리에 RESET API KEY값을 사용
    refresh_token = resToken['refresh_token']

    print("resToken : ",resToken)

    print("refresh_token :",refresh_token)

    result =  getAccessToken_refresh(refresh_token)
    print("result : ", result)

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
    u_id = 1
    payloadDict = dict({
            "object_type" : "text",
            "text" : text,
            "link" : {
                "web_url" : "http://192.168.0.115:5000/search",
                "mobile_web_url" : "http://192.168.0.115:5000/search"
             },
        "buttons": [
            {
                "title": title,
                "link": {
                    "web_url": "http://192.168.0.115:5000/search", 
                    "mobile_web_url": "http://192.168.0.115:5000/search",

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
    return access_token

# read(get or select) end

# update start
# update end

# delete start
####sounde_sensor delete data NodeMCU ==> Amazon DB
@sensor_api.route('/sensor/sound/delete', methods=['POST'])
def del_sound():
    sound = request.json
    db = sound['db']
    fq = sound['fq']

    cursor = mysql.get_db().cursor()
    cursor.execute("delete from sound_sensor where sound_db = (%s)", (db))

    mysql.get_db().commit()

    return jsonify({"message": "success"})

####tempature_sensor delete data NodeMCU ==> Amazon DB
@sensor_api.route('/sensor/temp/delete', methods=['POST'])
def del_temp_hum():

    tempature = request.json
    temp = tempature['temp']
    hum = tempature['hum']

    cursor = mysql.get_db().cursor()
    cursor.execute("delete from temp_sensor where temp = (%s)", (temp))

    mysql.get_db().commit()

    return jsonify({"message": "success"})

# delete end

# etc.
@sensor_api.route('/show_user')
def show_user():
    cursor = mysql.get_db().cursor()

    cursor.execute("select * from users" )
    user_table = cursor.fetchall()
    print(user_table)

    return 'ok'