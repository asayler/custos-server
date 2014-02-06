# -*- coding: utf-8 -*-

import base64
import uuid

import custos_db as db

VERSION = u'0.3-dev'

ARGS_AAS = u"aas"
ARGS_OVR = u"ovr"

STANZA_STAT = u"Status"
STANZA_GRPS = u"Groups"
STANZA_OBJS = u"Objects"
STANZA_VAL = u"Value"
STANZA_AAS = u"AccessAttributes"

RES_STATUS_ACCEPTED = u"accepted"
RES_STATUS_DENIED = u"denied"

_KEY_STATUS_ACCEPTED = u"accepted"
_KEY_STATUS_DENIED   = u"denied"
_KEY_STATUS_UNKNOWN  = u"unknown"

_ATTR_STATUS_ACCEPTED = u"accepted"
_ATTR_STATUS_DENIED   = u"denied"
_ATTR_STATUS_REQUIRED = u"required"
_ATTR_STATUS_IGNORED  = u"ignored"
_ATTR_STATUS_OPTIONAL = u"optional"

_ATTR_CLASS_IMPLICIT       = u"implicit"
_ATTR_TYPE_IMPLICIT_IP_SRC = u"ip_src"

_ATTR_CLASS_EXPLICIT    = u"explicit"
_ATTR_TYPE_EXPLICIT_PSK = u"psk"

_PERM_PRE_SRV = u"srv_"
_PERM_PRE_GRP = u"grp_"
_PERM_PRE_OBJ = u"obj_"

CXT_IP_SRC = u"ip_src"
CXT_USER = u"user"

_NO_VAL = None

def grp_list():

    return db.list_srv_grps()

def obj_list(grp_uuid):

    return db.list_grp_objs(grp_uuid)

def obj_get(obj_uuid):

    return db.get_obj_val(obj_uuid)

def create_cxt_AAs(cxt, echo):

    AAs_cxt = []

    # IP Source
    if cxt[CXT_IP_SRC] != None:
        attr = { u'Class': _ATTR_CLASS_IMPLICIT,
                 u'Type': _ATTR_TYPE_IMPLICIT_IP_SRC,
                 u'Value': base64.b64encode(cxt[CXT_IP_SRC] + '\0'),
                 u'Echo': echo }
        AAs_cxt.append(attr)

    return AAs_cxt

def check_perm(perm, AAs_pro, uuid=None, ovr=False):

    # Lookup ACS
    if perm.startswith(_PERM_PRE_SRV):
        acs = db.get_srv_ACS()
    elif perm.startswith(_PERM_PRE_GRP):
        acs = db.get_grp_ACS(uuid)
    elif perm.startswith(_PERM_PRE_OBJ):
        acs = db.get_obj_ACS(uuid)
    else:
        raise Exception("Unknown permission prefix")
    if acs is None:
        raise Exception("No ACS returned")

    print("acs = {:s}".format(acs))
    print("perm = {:s}".format(perm))

    # Lookup ACC
    acc = acs[perm]

    # Check ACC
    # TODO Provide smarter mutli-chain checking
    for chain in acc:

        AAs_req = [ db.get_attr_val(aa) for aa in chain ]
        AAs_out = check_AAs(AAs_req, AAs_pro)
        if AAs_out is None:
            raise Exception("No attributes returned")

        # Derive Pass/Fail
        stats = set([ aa[u'Status'] for aa in AAs_out ])
        if ((_ATTR_STATUS_DENIED in stats) or
            (_ATTR_STATUS_REQUIRED in stats)):
            success = False
        else:
            success = True
            break

    return (success, AAs_out)

def check_AAs(requested, provided):

    print("requested = {:s}".format(requested))
    print("provided = {:s}".format(provided))

    output = []
    unused = provided
    available = []

    # Process requested
    while (len(requested) > 0):
        r = requested.pop(0)

        # Search for match
        match = False
        available = unused + available
        unused = []
        while (len(available) > 0):
            p = available.pop(0)
            # Compare ID
            if ((p[u'Class'] == r[u'Class']) and
                (p[u'Type'] == r[u'Type'])):
                # Match
                match = True
                if(test_attr(r[u'Class'], r[u'Type'],
                             r[u'Value'], p[u'Value'])):
                    # Valid
                    stat = _ATTR_STATUS_ACCEPTED
                else:
                    # Invalid
                    stat = _ATTR_STATUS_DENIED
                out = create_attr_res(p, p[u'Echo'], stat)
                output.append(out)
                break;
            else:
                # No Match
                unused.append(p)

        # Add required
        if not match:
            stat = _ATTR_STATUS_REQUIRED
            out = create_attr_res(r, False, stat)
            output.append(out)

    # Add ignored
    available = unused + available
    unused = []
    while (len(available) > 0):
        p = available.pop(0)
        stat = _ATTR_STATUS_IGNORED
        out = create_attr_res(p, p[u'Echo'], stat)
        output.append(out)

    return output


def create_attr_res(attr, echo, stat):

    out = {}
    out[u'Class'] = attr[u'Class']
    out[u'Type'] = attr[u'Type']
    out[u'Echo'] = echo
    if echo:
        out[u'Value'] = attr[u'Value']
    else:
        out[u'Value'] = _NO_VAL
    out[u'Status'] = stat

    return out


def test_attr(cls, typ, val_a, val_b):

    a = base64.b64decode(val_a)
    b = base64.b64decode(val_b)

    if (cls == _ATTR_CLASS_EXPLICIT):
        if (typ == _ATTR_TYPE_EXPLICIT_PSK):
            return (a == b)
        else:
            raise Exception("Unknown attr type {:s} in class {:s}".format(cls, typ))
    else:
        raise Exception("Unknown attr class {:s}".format(cls))
