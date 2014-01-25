import base64
import uuid

import custos_db as db

_VERSION = '0.3-dev'

_TEST_KEY = "This is only a test!"

ARGS_AAS = "aas"
ARGS_OVR = "ovr"

STANZA_STAT = "Status"
STANZA_GRPS = "Groups"
STANZA_OBJS = "Objects"
STANZA_AAS = "AccessAttributes"

RES_STATUS_ACCEPTED = "accepted"
RES_STATUS_DENIED = "denied"

_KEY_STATUS_ACCEPTED = "accepted"
_KEY_STATUS_DENIED   = "denied"
_KEY_STATUS_UNKNOWN  = "unknown"

_ATTR_STATUS_ACCEPTED = "accepted"
_ATTR_STATUS_DENIED   = "denied"
_ATTR_STATUS_REQUIRED = "required"
_ATTR_STATUS_IGNORED  = "ignored"
_ATTR_STATUS_OPTIONAL = "optional"

_ATTR_CLASS_IMPLICIT       = "implicit"
_ATTR_TYPE_IMPLICIT_IP_SRC = "ip_src"

_ATTR_CLASS_EXPLICIT    = "explicit"
_ATTR_TYPE_EXPLICIT_PSK = "psk"

_PERM_PRE_SRV = "srv_"
_PERM_PRE_GRP = "grp_"
_PERM_PRE_OBJ = "obj_"

CXT_IP_SRC = "ip_src"
CXT_USER = "user"

_NO_VAL = None

_ENCODING = 'utf-8'


def create_cxt_AAs(cxt, echo):

    AAs_cxt = []

    # IP Source
    if cxt[CXT_IP_SRC] != None:
        attr = { 'Class': _ATTR_CLASS_IMPLICIT,
                 'Type': _ATTR_TYPE_IMPLICIT_IP_SRC,
                 'Value': base64.b64encode(cxt[CXT_IP_SRC] + '\0'),
                 'Echo': echo }
        AAs_cxt.append(attr)

    return AAs_cxt

def check_perm(perm, AAs_in, uuid=None, ovr=False):

    # Lookup ACS
    if perm.startswith(_PERM_PRE_SRV):
        acs = db.get_srv_ACS()
    elif passperm.startswith(_PERM_PRE_GRP):
        acs = db.get_grp_ACS(uuid)
    elif passperm.startswith(_PERM_PRE_OBJ):
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
    AAs_out = check_AAs(acc, AAs_in)
    if AAs_out is None:
        raise Exception("No attributes returned")

    # Derive Pass/Fail
    stats = set(attr['Status'] for attr in out_attrs)
    if ((_ATTR_STATUS_DENIED in stats) or
        (_ATTR_STATUS_REQUIRED in stats)):
        success = False
    else:
        success = True

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
            if ((p['Class'] == r['Class']) and
                (p['Type'] == r['Type'])):
                # Match
                match = True
                if(test_attr(r['Class'], r['Type'],
                             r['Value'], p['Value'])):
                    # Valid
                    stat = _ATTR_STATUS_ACCEPTED
                else:
                    # Invalid
                    stat = _ATTR_STATUS_DENIED
                out = create_attr_res(p, p['Echo'], stat)
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
        out = create_attr_res(p, p['Echo'], stat)
        output.append(out)

    return output


def create_attr_res(attr, echo, stat):

    out = {}
    out['Class'] = attr['Class']
    out['Type'] = attr['Type']
    out['Echo'] = echo
    if echo:
        out['Value'] = attr['Value']
    else:
        out['Value'] = _NO_VAL
    out['Status'] = stat

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


def process_keys_get(req, context=None, source=None):

    res = {}
    res['Source'] = source
    res['Version'] = _VERSION
    res['ResID'] = str(uuid.uuid1())
    res['ReqID'] = req['ReqID']
    res['Status'] = _RES_STATUS_ACCEPTED

    pvd_attrs = req['Attrs']
    cxt_attrs = []

    # Process Context
    if (context['source_ip'] != None):
        attr_ip = {}
        attr_ip['Class'] = 'implicit'
        attr_ip['Type'] = 'ip_source'
        attr_ip['Index'] = 0
        attr_ip['Value'] = base64.b64encode(context['source_ip'] + '\0')
        attr_ip['Echo'] = True
        attr_ip['Status'] = _ATTR_STATUS_IGNORED
        cxt_attrs.append(attr_ip)

    # Process Key for Read Access
    res_attrs = []
    res_keys = []
    for key in req['Keys']:

        key_out = {}
        key_out['UUID'] = key['UUID']
        key_out['Revision'] = key['Revision']

        acls = db.get_ACLS_read(key['UUID'].encode(_ENCODING))

        # Handel Missing Key
        if acls is None:
            key_out['Echo'] = False
            key_out['Value'] = _NO_VAL
            key_out['Status'] = _KEY_STATUS_UNKNOWN
            continue

        # Handle Missing ACLS
        if (len(acls) == 0):
            raise Exception("acls list must not be empty")

        # Process ACLS
        out_attrs_tups = []
        for acl in acls:

            if acl is None:
                raise Exception("acl must not be None")

            # Get Attrs from DB
            rqd_attrs = []
            for attr_uuid in acl:
                attr = db.get_attr_val(attr_uuid)
                if attr is None:
                    raise Exception("missing attr value")
                rqd_attrs.append(attr)

            # Process Attrs
            out_attrs = process_attrs(rqd_attrs, (pvd_attrs + cxt_attrs))
            if out_attrs is None:
                raise Exception("No attributes returned")

            # Find Chain Success
            stats = set(attr['Status'] for attr in out_attrs)
            if ((_ATTR_STATUS_DENIED in stats) or
                (_ATTR_STATUS_REQUIRED in stats)):
                success = False
            else:
                success = True

            # Find Chain Length
            length = 0
            for attr in out_attrs:
                if (attr['Status'] == _ATTR_STATUS_ACCEPTED):
                    length += 1
                else:
                    break

            # Save
            out_attrs_tups.append((out_attrs, success, length))

        # Calculate Response
        suc_tup = None
        max_tup = out_attrs_tups[0]
        for tup in out_attrs_tups:
            if (tup[1] == True):
                suc_tup = tup
                break;
            if (tup[2] > max_tup[2]):
                max_tup = tup

        # Process Response
        if suc_tup is not None:
            val = db.get_key_val(key['UUID'].encode(_ENCODING))
            if val is None:
                raise Exception("missing key value")
            key_out['Echo'] = True
            key_out['Value'] = val
            key_out['Status'] = _KEY_STATUS_ACCEPTED
            res_attrs.append(suc_tup[0])
        else:
            key_out['Echo'] = False
            key_out['Value'] = _NO_VAL
            key_out['Status'] = _KEY_STATUS_DENIED
            res_attrs.append(max_tup[0])

        res_keys.append(key_out)

    # TODO Merge res_attrs
    res['Attrs'] = res_attrs[0]
    res['Keys'] = res_keys

    print(res)
    return res
