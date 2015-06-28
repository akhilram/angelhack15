<?php
$token = '118126117-wsRD0LXpbiyNju7QsTzvDBZy7WYIYvKMfKY4CX0H';
$token_secret = 'OdYxCwfSZIWFhmz6SZlhvSn1hcdYYkPaYZqvddgCzuaFc';
$consumer_key = '0VNlqnyujCDdgBzeGZ2x9DrzV';
$consumer_secret = 'rniZ0WitGr7cOLV5oz6vV6mzLz6F9sj4Kra4MWN5LytnviJDb9';

$host = 'api.twitter.com';
$method = 'GET';
$path = '/1.1/statuses/user_timeline.json';

$query = array(
    'screen_name' => 'rashmeshrk',
    'count' => '10'
);

$oauth = array(
    'oauth_consumer_key' => $consumer_key,
    'oauth_token' => $token,
    'oauth_nonce' => (string)mt_rand(),
    'oauth_timestamp' => time(),
    'oauth_signature_method' => 'HMAC-SHA1',
    'oauth_version' => '1.0'
);

$oauth = array_map("rawurlencode", $oauth);
$query = array_map("rawurlencode", $query);

$arr = array_merge($oauth, $query);

asort($arr);
ksort($arr);

$querystring = urldecode(http_build_query($arr, '', '&'));

$url = "https://$host$path";


$base_string = $method . "&" . rawurlencode($url) . "&" . rawurlencode($querystring);


$key = rawurlencode($consumer_secret) . "&" . rawurlencode($token_secret);


$signature = rawurlencode(base64_encode(hash_hmac('sha1', $base_string, $key, true)));


$url .= "?" . http_build_query($query);
$url = str_replace("&amp;", "&", $url);

$oauth['oauth_signature'] = $signature;
ksort($oauth);

function add_quotes($str)
{
    return '"' . $str . '"';
}

$oauth = array_map("add_quotes", $oauth);


$auth = "OAuth " . urldecode(http_build_query($oauth, '', ', '));


$options = array(CURLOPT_HTTPHEADER => array("Authorization: $auth"),
    CURLOPT_HEADER => false,
    CURLOPT_URL => $url,
    CURLOPT_RETURNTRANSFER => true,
    CURLOPT_SSL_VERIFYPEER => false);


$feed = curl_init();
curl_setopt_array($feed, $options);
$json = curl_exec($feed);
curl_close($feed);

$twitter_data = json_decode($json);


$json_result = json_decode($json);

$feeds = [];
foreach ($json_result as &$result) {
    $feeds[] .= $result->text;
}

echo json_encode($feeds);

?>