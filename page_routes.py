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

routes_api = Blueprint('routes_api', __name__)

# join page
@routes_api.route('/join')
def join():
    title = 'join'
    return render_template('join.html', title = title)

# search page
@routes_api.route('/search')
def search():
    title = 'search'
    return render_template('search.html', title = title)

@routes_api.route('/show')
def show():
    title = 'Baby status'
    args = request.args
    u_id = args.get('u_id')

    return render_template('display_sensor.html', u_id=u_id, title= title)

@routes_api.route('/crawler')
def crawler():
    title = 'crawler'
    return render_template('select_crawler.html', title = title)