from flask import Flask, render_template, jsonify, request, g
import json
import copy
import hashlib

app = Flask(__name__)
app.debug = True

def decode_json_req(json_req_in, json_chk_in):

    if (json_chk_in != None):
        chk = json.loads(json_chk_in)
        m = hashlib.md5()
        m.update(json_req_in)
        if(m.hexdigest() != chk['md5']):
            # TODO Raise/return real error
            print("ERROR: md5 Mismatch!")
            print("calc md5 = " + m.hexdigest())
            print("sent md5 = " + chk['md5'])

    req = json.loads(json_req_in)

    return req


@app.before_request
def before_request():
    pass


@app.route("/", methods=['GET'])
def endpoint_root():
    return "This is the python Custos server implementation."


@app.route("/keys", methods=['GET'])
def endpoint_keys():

    args_req_json = request.args.get('req')
    args_chk_json = request.args.get('chk')

    res = decode_json_req(args_req_json, args_chk_json)

    return jsonify(res)


@app.route("/echo", methods=['GET'])
def endpoint_echo():

    args_req_json = request.args.get('req')
    args_chk_json = request.args.get('chk')

    res = decode_json_req(args_req_json, args_chk_json)

    return jsonify(res)


if __name__ == "__main__":
    app.run()
