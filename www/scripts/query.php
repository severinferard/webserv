<?php
if (isset($_GET['query'])) {
    echo $_GET['query'];
} else {
    echo "'query' variable not set";
}
?>