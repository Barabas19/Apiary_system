<?php
if(count($_GET) > 0) {
    $_GET['method'] = 'GET';
    echo json_encode($_GET);
}

if(count($_POST) > 0) {
    $_POST['method'] = 'POST';
    echo json_encode($_POST);
}
?>
