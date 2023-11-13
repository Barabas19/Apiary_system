<?php
if(count($_POST) > 0) {
    $_POST['method'] = 'POST';
    $config = [];
    $config['burst'] = 3600;
    echo json_encode($config);
}
?>
