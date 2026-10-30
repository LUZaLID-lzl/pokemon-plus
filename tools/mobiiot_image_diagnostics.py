#!/usr/bin/env python3
"""Run the installed image client unchanged, reporting only safe error categories.

The skill's main and retry loop own every request, model choice, and retry.
Never print the response body, server messages, URLs, request fields, or keys.
"""
from pathlib import Path
import json
import runpy
import sys
import urllib.error

client=runpy.run_path(str(Path.home()/'.codex/skills/mobiiot-imagegen/scripts/imagegen.py'),run_name='mobiiot_client')

def operation():
    try:
        return client['main']()
    except urllib.error.HTTPError as error:
        category='unclassified';parameter='unspecified'
        try:
            data=json.loads(error.read(65536))
            detail=data.get('error',data) if isinstance(data,dict) else {}
            if isinstance(detail,str):message=detail.lower()
            elif isinstance(detail,dict):
                message=str(detail.get('message','')).lower()
                if detail.get('param') in ('model','prompt','image','image[]','output_format','size','quality'):
                    parameter=detail['param']
            else:message=''
            rules=[
                ('authentication',('invalid api key','invalid_api_key','unauthorized','authentication')),
                ('quota_or_rate_limit',('quota','rate limit','rate_limit','too many requests')),
                ('no_available_upstream',('no available','no healthy','no capacity','no channel','no account')),
                ('content_policy',('content policy','content_policy','safety','moderation')),
                ('image_input',('invalid image','image format','image size','image dimensions','image resolution','decode image','unsupported image')),
                ('multipart_input',('multipart','image[]','missing image','file upload')),
                ('model_compatibility',('unsupported model','model not found','model_not_found','does not support','unknown model')),
                ('upstream_request_failure',('upstream','bad request','request failed')),
            ]
            category=next((name for name,terms in rules if any(term in message for term in terms)),'unclassified')
        except (ValueError,OSError,AttributeError,TypeError):
            pass
        print(f'Request diagnostic: HTTP {error.code}; category={category}; parameter={parameter}',file=sys.stderr,flush=True)
        raise

if __name__=='__main__':
    sys.exit(client['run_with_retries'](operation))
