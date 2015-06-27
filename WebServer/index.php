
<?php
$post_data = 'Think back to our talk earlier about Cross-Origin Resource Sharing (CORS). Remember how it introduces a security vulnerability? We\'re going to work to close that as tightly as possible here by tying an Origin to a unique API Key. This means that only known and allowed external hosts will be able to connect to our API service through a pairing of their domain name and a uniquely generated API Key. For the purposes of this example I\'m going to leave some of the code to verify the API Key abstracted out. Additionally our API will require a unique token in every request to verify the User.';
$post_data = json_encode(array('item' => $post_data), JSON_FORCE_OBJECT);
echo $post_data;
?>