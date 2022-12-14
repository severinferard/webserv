from pathlib import Path
import requests
import pytest
import subprocess
import random
import string

HOST = "localhost"
PORT = 8888
BASE_URL = f"http://{HOST}:{PORT}"

WWW_DIR = Path(__file__).resolve().parent.parent / 'www/test/pytest'
DATA_DIR = Path(__file__).resolve().parent.parent / 'www/data'
SKIP_LONGER_TESTS = True
REQUEST_MAX_TIMEOUT = 65

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

@pytest.fixture
def create_1_billion_file():
    yield subprocess.run(f"LC_ALL=C tr -dc A-Za-z0-9 </dev/urandom | head -c 1000000000 > {WWW_DIR / 'onebillion.txt'}", shell=True)
    subprocess.run(f"rm -rf {WWW_DIR / 'onebillion.txt'}", shell=True)

@pytest.fixture(scope='session', autouse=True)
def clean_www_data():
    yield
    # Will be executed after the last test
    subprocess.run(f"rm -rf {DATA_DIR / '*'}", shell=True)
    subprocess.run(f"touch {DATA_DIR / '.hidden'}", shell=True)

@pytest.fixture
def create_tiny_deleteme_file():
    yield subprocess.run(f"cp -f {WWW_DIR / 'tiny_lorem.txt'} {WWW_DIR / 'tiny_deleteme.txt'}", shell=True)

@pytest.fixture
def create_short_deleteme_file():
    yield subprocess.run(f"cp -f {WWW_DIR / 'short_lorem.txt'} {WWW_DIR / 'short_deleteme.txt'}", shell=True)

@pytest.fixture
def create_medium_deleteme_file():
    yield subprocess.run(f"cp -f {WWW_DIR / 'medium_lorem.txt'} {WWW_DIR / 'medium_deleteme.txt'}", shell=True)

@pytest.fixture
def create_long_deleteme_file():
    yield subprocess.run(f"cp -f {WWW_DIR / 'long_lorem.txt'} {WWW_DIR / 'long_deleteme.txt'}", shell=True)

@pytest.fixture
def create_deleteme_file():
    yield subprocess.run(f"echo my fate is to be deleted in a python test... > {WWW_DIR / 'deleteme.txt'}", shell=True)

def random_filename():
    return ''.join(random.choice(string.ascii_lowercase + string.digits) for _ in range(8))
