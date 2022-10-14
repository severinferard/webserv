from pathlib import Path
import requests
import pytest
import subprocess

HOST = "localhost"
PORT = 8888
BASE_URL = f"http://{HOST}:{PORT}"

WWW_DIR = Path(__file__).resolve().parent.parent / 'www/test/pytest'

def pretty_print_request(req):
    print('{}\n{}\r\n{}\r\n\r\n{}'.format(
        '-----------Request-----------',
        req.method + ' ' + req.url,
        '\r\n'.join('{}: {}'.format(k, v) for k, v in req.headers.items()),
        req.body,
    ))

def pretty_print_response(res: requests.Response):
    print('{}\n{}\r\n{}\r\n\r\n{}'.format(
        '-----------Response-----------',
        res.url + ' ' +  str(res.status_code),
        '\r\n'.join('{}: {}'.format(k, v) for k, v in res.headers.items()),
        str(res.content),
    ))

@pytest.fixture
def create_1_million_file():
    yield subprocess.run(f"LC_ALL=C tr -dc A-Za-z0-9 </dev/urandom | head -c 1000000 > {WWW_DIR / 'onemillion.txt'}", shell=True)
    subprocess.run(f"rm -rf {WWW_DIR / 'onemillion.txt'}", shell=True)