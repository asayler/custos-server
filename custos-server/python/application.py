#!/usr/bin/env python
# -*- coding: utf-8 -*-

from flask import Flask, jsonify, request
import json
import copy
import hashlib

import custos

app = Flask(__name__)
app.debug = True

_ENCODING = 'utf-8'

_ENDPOINT_PERM = { u"grp_list": u"srv_grp_list",
                   u"obj_list": u"grp_obj_list",
                   u"obj_get":  u"obj_read" }


def _auth_request(req, endpoint, uuid=None, ovr=False):

    # Extract Args
    args_aa_json = req.args.get(custos.ARGS_AAS)
    args_ovr_json = req.args.get(custos.ARGS_OVR)

    # Decode Args
    if args_aa_json:
        args_aa = json.loads(args_aa_json)
    else:
        args_aa = []

    # Get Context
    cxt = {}
    cxt[custos.CXT_IP_SRC] = request.remote_addr
    cxt[custos.CXT_USER] = request.remote_user

    # Append Context
    AAs_in = args_aa + custos.create_cxt_AAs(cxt, True)

    # Check Permission
    return custos.check_perm(_ENDPOINT_PERM[endpoint], AAs_in, uuid, ovr)


@app.before_request
def before_request():
    pass


@app.route("/", methods=['GET'])
def endpoint_root():
    return "Python Custos Server Implementation Version {:s}".format(custos.VERSION)


@app.route("/grp", methods=['GET'])
def endpoint_grp_list():
    endpt = 'grp_list'

    # Setup Response
    res = {}

    # Auth Request
    success, AAs_out = _auth_request(request, endpt)
    res[custos.STANZA_AAS] = AAs_out

    # Process Request
    if success:
        res[custos.STANZA_STAT] = custos.RES_STATUS_ACCEPTED
        res[custos.STANZA_GRPS] = custos.grp_list()
    else:
        res[custos.STANZA_STAT] = custos.RES_STATUS_DENIED
        res[custos.STANZA_GRPS] = None

    return jsonify(res)


@app.route("/grp/<grp_uuid>/obj", methods=['GET'])
def endpoint_obj_list(grp_uuid):
    endpt = 'obj_list'

    # Setup Response
    res = {}

    # Auth Request
    success, AAs_out = _auth_request(request, endpt, grp_uuid)
    res[custos.STANZA_AAS] = AAs_out

    # Process Request
    if success:
        res[custos.STANZA_STAT] = custos.RES_STATUS_ACCEPTED
        res[custos.STANZA_OBJS] = custos.obj_list(grp_uuid)
    else:
        res[custos.STANZA_STAT] = custos.RES_STATUS_DENIED
        res[custos.STANZA_OBJS] = None

    return jsonify(res)


@app.route("/grp/<grp_uuid>/obj/<obj_uuid>", methods=['GET'])
def endpoint_obj_get(grp_uuid, obj_uuid):
    endpt = 'obj_get'

    # Setup Response
    res = {}

    # TODO Verify object in group

    # Auth Request
    success, AAs_out = _auth_request(request, endpt, obj_uuid)
    res[custos.STANZA_AAS] = AAs_out

    # Process Request
    if success:
        res[custos.STANZA_STAT] = custos.RES_STATUS_ACCEPTED
        res[custos.STANZA_OBJS] = custos.obj_get(obj_uuid)
    else:
        res[custos.STANZA_STAT] = custos.RES_STATUS_DENIED
        res[custos.STANZA_OBJS] = None

    return jsonify(res)


if __name__ == "__main__":
    app.run()
