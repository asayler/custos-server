from flask import Flask, jsonify, request, g
import json
import copy
import hashlib

import custos

app = Flask(__name__)
app.debug = True

_ENCODING = 'utf-8'

_ENDPOINT_PERM = { "grp_get": "srv_grp_list",
                   "grp_post": "srv_grp_create" }

def decode_json_req(json_req_in, json_chk_in=None):

    if (json_chk_in != None):
        chk = json.loads(json_chk_in)
        m = hashlib.md5()
        m.update(json_req_in.encode(_ENCODING))
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


@app.route("/grp", methods=['GET'])
def endpoint_grp_get():
    fname = 'grp_get'

    # Extract Args
    args_aa_json = request.args.get(custos.ARGS_AAS)
    args_ovr_json = request.args.get(custos.ARGS_OVR)

    # Decode Args
    if args_aa_json != None:
        args_aa = decode_json_req(args_aa_json)
    else:
        args_aa = []

    # Get Context
    cxt = {}
    cxt[custos.CXT_IP_SRC] = request.remote_addr
    cxt[custos.CXT_USER] = request.remote_user

    # Append Context
    AAs_in = args_aa + custos.create_cxt_AAs(cxt, True)

    # Setup Response
    res = {}

    # Check Permission
    success, AAs_out = custos.check_perm(_ENDPOINT_PERM[fname], AAs_in)
    res[custos.STANZA_AAS] = AAs_out

    # Process Request
    if success:
        res[custos.STANZA_STAT] = custos.RES_STATUS_ACCEPTED
        res[custos.STANZA_GRPS] = custos.grp_list()
    else:
        res[custos.STANZA_STAT] = custos.RES_STATUS_DENIED
        res[custos.STANZA_GRPS] = None

    return jsonify(res)


@app.route("/keys", methods=['GET'])
def endpoint_keys():

    args_req_json = request.args.get('req')
    args_chk_json = request.args.get('chk')

    req = decode_json_req(args_req_json, args_chk_json)

    cxt = {}
    cxt['source_ip'] = request.remote_addr
    cxt['source_user'] = request.remote_user

    src = request.url_root

    res = custos.process_keys_get(req, context=cxt, source=src)

    return jsonify(res)


@app.route("/echo", methods=['GET'])
def endpoint_echo():

    args_req_json = request.args.get('req')
    args_chk_json = request.args.get('chk')

    res = decode_json_req(args_req_json, args_chk_json)

    return jsonify(res)


if __name__ == "__main__":
    app.run()
