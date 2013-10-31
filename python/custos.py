import base64
import uuid

_VERSION = '0.1-dev'

_TEST_KEY = "This is only a test!"

_RES_STATUS_ACCEPTED = "accepted"
_KEY_STATUS_ACCEPTED = "accepted"
_ATTR_STATUS_ACCEPTED = "accepted"
_ATTR_STATUS_IGNORED = "ignored"

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
        attr_out['Value'] = ""
        attr_out['Echo'] = attr_in['Echo']
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
        key_out['Value'] = base64.b64encode(_TEST_KEY + '\0')
        key_out['Echo'] = key_in['Echo']
        key_out['Status'] = _KEY_STATUS_ACCEPTED
        res['Keys'].append(key_out)

    return res
