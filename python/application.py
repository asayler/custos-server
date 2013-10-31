from flask import Flask, render_template, jsonify, request, g
import json

app = Flask(__name__)
app.debug = True


@app.before_request
def before_request():
    pass


@app.route("/", methods=['GET'])
def endpoint_root():
    return "This is the python Custos server implementation."


@app.route("/keys", methods=['GET'])
def endpoint_keys():
    args_req_json = request.args.get('req')
    req = json.loads(args_req_json)
    res = req
    return jsonify(res)


@app.route("/echo", methods=['GET'])
def endpoint_echo():
    args_req_json = request.args.get('req')
    req = json.loads(args_req_json)
    res = req
    return jsonify(res)


if __name__ == "__main__":
    app.run()
