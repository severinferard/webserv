import re
import sre_compile
import requests
import socket
import pytest
from pathlib import Path
import random
import string
import os
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


def test_GET_with_autoindex_check_status():
    response = requests.get(BASE_URL + "/with-autoindex")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200

def test_GET_with_autoindex_check_content_type():
    response = requests.get(BASE_URL + "/with-autoindex")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.headers["Content-Type"] == 'text/html'

def test_GET_with_autoindex_check_body():
    response = requests.get(BASE_URL + "/with-autoindex")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert "<h1>Index of /with-autoindex</h1>" in str(response.content)
    assert '<a href="bar">bar</a>' in str(response.content)
    assert '<a href="FOOlder/">FOOlder/</a>' in str(response.content)

def test_GET_with_autoindex_disabled_expect_404():
    response = requests.get(BASE_URL + "/no-autoindex")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 404
    
def test_GET_with_index_check_status():
    response = requests.get(BASE_URL + "/with-index")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200

def test_GET_with_index_check_body():
    response = requests.get(BASE_URL + "/with-index")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert "Yup ! That's the index !" in str(response.content)

def test_GET_with_file_that_exist_tiny():
    response = requests.get(BASE_URL + "/tiny_lorem.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert len(response.content) == 26

def test_GET_with_file_that_exist_short():
    response = requests.get(BASE_URL + "/short_lorem.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert len(response.content) == 896

def test_GET_with_file_that_exist_medium():
    response = requests.get(BASE_URL + "/medium_lorem.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert len(response.content) == 9685

def test_GET_with_file_that_exist_long():
    response = requests.get(BASE_URL + "/long_lorem.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert len(response.content) == 96187

@pytest.fixture
def create_1_million_file():
    yield subprocess.run(f"LC_ALL=C tr -dc A-Za-z0-9 </dev/urandom | head -c 1000000 > {WWW_DIR / 'onemillion.txt'}", shell=True)
    subprocess.run(f"rm -rf {WWW_DIR / 'onemillion.txt'}", shell=True)

def test_GET_with_file_that_exist_1_million_bytes(create_1_million_file):
    response = requests.get(BASE_URL + "/onemillion.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert len(response.content) == 1_000_000
    
@pytest.fixture
def create_1_billion_file():
    yield subprocess.run(f"LC_ALL=C tr -dc A-Za-z0-9 </dev/urandom | head -c 1000000000 > {WWW_DIR / 'onebillion.txt'}", shell=True)
    subprocess.run(f"rm -rf {WWW_DIR / 'onebillion.txt'}", shell=True)

@pytest.mark.skip(reason="Too long... Must be tested during correction")
def test_GET_with_file_that_exist_1_billion_bytes(create_1_billion_file):
    response = requests.get(BASE_URL + "/onebillion.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert len(response.content) == 1_000_000_000

def test_GET_with_file_that_does_not_exist_check_status():
    response = requests.get(BASE_URL + "/toto")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 404

def test_GET_with_file_that_does_not_exist_check_page():
    response = requests.get(BASE_URL + "/toto")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.content == b"<html>\n    <h1>Webserv</h1>\n    <h2>404 Not Found</h2>\n</html>"

def test_GET_line_by_line_valid():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"GET / HTTP/1.1\r\n")
    conn.send(f"Host: {HOST}:{PORT}\r\n".encode())
    conn.send(f"User-Agent: curl/7.77.0\r\n".encode())
    conn.send(b"\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 200 OK"

def test_GET_line_by_line_bad_method_expect_400_1():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"42 / HTTP/1.1\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 400 Bad Request"

def test_GET_line_by_line_bad_method_expect_400_2():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"GET/ HTTP/1.1\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 400 Bad Request"

def test_GET_line_by_line_unsupported_method_expect_501():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"OPTIONS / HTTP/1.1\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 501 Not Implemented"

def test_GET_line_by_line_unsupported_http_version_expect_505_1():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"GET / HTTP/2.0\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 505 HTTP Version Not Supported"

def test_GET_line_by_line_unsupported_http_version_expect_505_2():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"GET / HPPT\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 505 HTTP Version Not Supported"

def test_GET_line_by_line_invalid_header_format_expect_400_1():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"GET / HTTP/1.1\r\n")
    conn.send(f"Host: {HOST}:{PORT}\r\n".encode())
    conn.send(f"User-Agent curl/7.77.0\r\n".encode()) # missing ':'
    conn.send(b"\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 400 Bad Request"

def test_GET_line_by_line_invalid_header_format_expect_400_2():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"GET / HTTP/1.1\r\n")
    conn.send(f"Host: {HOST}:{PORT}\r\n".encode())
    conn.send(f": curl/7.77.0\r\n".encode()) # No fieldname 
    conn.send(b"\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 400 Bad Request"

@pytest.mark.skip(reason="Not Implemented")
def test_GET_line_by_line_timeout():
    pass

    



