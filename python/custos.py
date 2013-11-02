import base64
import uuid

import custos_db as db

_VERSION = '0.1-dev'

_TEST_KEY = "This is only a test!"

_RES_STATUS_ACCEPTED = "accepted"

_KEY_STATUS_ACCEPTED = "accepted"
_KEY_STATUS_DENIED = "denied"
_KEY_STATUS_UNKNOWN = "unknown"

_ATTR_STATUS_ACCEPTED = "accepted"
_ATTR_STATUS_DENIED = "denied"
_ATTR_STATUS_REQUIRED = "required"
_ATTR_STATUS_IGNORED = "ignored"

_NO_VAL = ""

_ENCODING = 'utf-8'

def process_keys_get(req, context=None, source=None):

    res = {}
    res['Source'] = source
    res['Version'] = _VERSION
    res['ResID'] = str(uuid.uuid1())
    res['ReqID'] = req['ReqID']
    res['Status'] = _RES_STATUS_ACCEPTED
    res['Attrs'] = []
    res['Keys'] = []

    for attr_in in req['Attrs']:
        attr_out = {}
        attr_out['Class'] = attr_in['Class']
        attr_out['Type'] = attr_in['Type']
        attr_out['Index'] = attr_in['Index']
        attr_out['Value'] = attr_in['Value']
        attr_out['Echo'] = True
        attr_out['Status'] = _ATTR_STATUS_IGNORED
        res['Attrs'].append(attr_out)

    if (context['source_ip'] != None):
        attr_ip = {}
        attr_ip['Class'] = 'implicit'
        attr_ip['Type'] = 'ip_source'
        attr_ip['Index'] = 0
        attr_ip['Value'] = base64.b64encode(context['source_ip'] + '\0')
        attr_ip['Echo'] = True
        attr_ip['Status'] = _ATTR_STATUS_IGNORED
        res['Attrs'].append(attr_ip)

    for key_in in req['Keys']:

        key_out = {}
        key_out['UUID'] = key_in['UUID']
        key_out['Revision'] = key_in['Revision']
        key_out['Echo'] = key_in['Echo']

        acls = db.get_ACLS_read(key_out['UUID'].encode(_ENCODING))
        acls_tested = []
        for acl in acls:
            tested = []
            for attr_uuid in acl:
                attr = db.get_attr_val(attr_uuid)
                matched = False
                for attr_out in res['Attrs']:
                    if ((attr['Class'] == attr_out['Class']) and
                        (attr['Type'] == attr_out['Type']) and
                        (attr['Index'] == attr_out['Index'])):
                        matched = True
                        val_a = base64.b64decode(attr['Value'])
                        val_b = base64.b64decode(attr_out['Value'])
                        if (val_a == val_b):
                            tested += ['pass']
                            attr_out['Status'] = _ATTR_STATUS_ACCEPTED
                        else:
                            tested += ['fail']
                            attr_out['Status'] = _ATTR_STATUS_DENIED
                        break
                if not matched:
                    tested += ['fail']
                    attr['Value'] = _NO_VAL
                    attr['Echo'] = False
                    attr['Status'] = _ATTR_STATUS_REQUIRED
                    res['Attrs'].append(attr)
            acls_tested += [tested]

        grant = False
        for tested in acls_tested:
            access = set(tested)
            if ((len(access) == 1) and
                ('pass' in access)):
                grant = True
                break

        if grant:
            val = db.get_key_val(key_out['UUID'].encode(_ENCODING))
            if val:
                key_out['Value'] = val
                key_out['Status'] = _KEY_STATUS_ACCEPTED
            else:
                key_out['Value'] = _NO_VAL
                key_out['Status'] = _KEY_STATUS_UNKNOWN
        else:
            key_out['Value'] = _NO_VAL
            key_out['Status'] = _KEY_STATUS_DENIED

        res['Keys'].append(key_out)

    return res
