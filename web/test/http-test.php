<?php
foreach($_GET as $key => $value) {
    echo 'GET ' . $key . '=' . $value . '<br>';
}
foreach($_POST as $key => $value) {
    echo 'POST ' . $key . '=' . $value . '<br>';
}
?>
