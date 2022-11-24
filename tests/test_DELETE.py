import requests
import socket
import pytest
from pathlib import Path
import subprocess
from os.path import exists

from common import *

def test_DELETE_root_check_status():
    response = requests.delete(BASE_URL + "/")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 405

def test_DELETE_with_file_that_exist_tiny(create_tiny_deleteme_file):
    response = requests.delete(BASE_URL + "/tiny_deleteme.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200
    assert not exists(BASE_URL + "/tiny_deleteme.txt")

def test_DELETE_with_file_that_exist_short(create_short_deleteme_file):
    response = requests.delete(BASE_URL + "/short_deleteme.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200
    assert not exists(BASE_URL + "/short_deleteme.txt")

def test_DELETE_with_file_that_exist_medium(create_medium_deleteme_file):
    response = requests.delete(BASE_URL + "/medium_deleteme.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200
    assert not exists(BASE_URL + "/medium_deleteme.txt")

def test_DELETE_with_file_that_exist_long(create_long_deleteme_file):
    response = requests.delete(BASE_URL + "/long_deleteme.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200
    assert not exists(BASE_URL + "/long_deleteme.txt")



def test_DELETE_with_file_that_exist_1_million_bytes(create_1_million_file):
    response = requests.delete(BASE_URL + "/onemillion.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200
    assert not exists(BASE_URL + "onemillion.txt")
    

@pytest.mark.skipif(SKIP_LONGER_TESTS, reason="Too long... Must be tested during correction")
def test_DELETE_with_file_that_exist_1_billion_bytes(create_1_billion_file):
    response = requests.delete(BASE_URL + "/onebillion.txt")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 200
    assert not exists(BASE_URL + "onebillion.txt")

def test_DELETE_with_file_that_does_not_exist_check_status():
    response = requests.delete(BASE_URL + "/toto")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.status_code == 404

def test_DELETE_with_file_that_does_not_exist_check_page():
    response = requests.delete(BASE_URL + "/toto")
    pretty_print_request(response.request)
    pretty_print_response(response)
    assert response.content == b"<html>\n    <h1>Webserv</h1>\n    <h2>404 Not Found</h2>\n</html>"

def test_DELETE_line_by_line_valid(create_deleteme_file):
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"DELETE /deleteme.txt HTTP/1.1\r\n")
    conn.send(f"Host: {HOST}:{PORT}\r\n".encode())
    conn.send(f"User-Agent: curl/7.77.0\r\n".encode())
    conn.send(b"\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 200 OK"

def test_DELETE_line_by_line_bad_method_expect_400_1():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"42 / HTTP/1.1\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 400 Bad Request"

def test_DELETE_line_by_line_bad_method_expect_400_2():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"DELETE/ HTTP/1.1\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 400 Bad Request"

def test_DELETE_line_by_line_unsupported_method_expect_501():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"OPTIONS / HTTP/1.1\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 501 Not Implemented"

def test_DELETE_line_by_line_unsupported_http_version_expect_505_1():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"DELETE / HTTP/2.0\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 505 HTTP Version Not Supported"

def test_DELETE_line_by_line_unsupported_http_version_expect_505_2():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"DELETE / HPPT\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 505 HTTP Version Not Supported"

def test_DELETE_line_by_line_invalid_header_format_expect_400_1():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"DELETE / HTTP/1.1\r\n")
    conn.send(f"Host: {HOST}:{PORT}\r\n".encode())
    conn.send(f"User-Agent curl/7.77.0\r\n".encode()) # missing ':'
    conn.send(b"\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 400 Bad Request"

def test_DELETE_line_by_line_invalid_header_format_expect_400_2():
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((HOST, PORT))
    conn.send(b"DELETE / HTTP/1.1\r\n")
    conn.send(f"Host: {HOST}:{PORT}\r\n".encode())
    conn.send(f": curl/7.77.0\r\n".encode()) # No fieldname 
    conn.send(b"\r\n")
    res = conn.recv(1000)
    print(res.decode())
    assert res.decode().split("\r\n")[0] == "HTTP/1.1 400 Bad Request"

