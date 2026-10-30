#!/usr/bin/env python3
from http.server import ThreadingHTTPServer,SimpleHTTPRequestHandler
from pathlib import Path
import argparse,functools
p=argparse.ArgumentParser();p.add_argument('--port',type=int,default=4173);a=p.parse_args()
root=Path(__file__).resolve().parents[1]
class ResourceHandler(SimpleHTTPRequestHandler):
 def end_headers(self):
  # Generated files can change between development reloads. Avoid retaining a
  # failed/obsolete module or thumbnail response under the same resource URL.
  self.send_header('Cache-Control','no-store')
  super().end_headers()
class ResourceServer(ThreadingHTTPServer):
 request_queue_size=128
print(f'资源工坊: http://127.0.0.1:{a.port}/web/',flush=True)
ResourceServer(('127.0.0.1',a.port),functools.partial(ResourceHandler,directory=str(root))).serve_forever()
