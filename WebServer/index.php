<?php

function financeAPI() {

	$query = 'http://pipes.yahoo.com/pipes/pipe.run?_id=2FV68p9G3BGVbc7IdLq02Q&_render=json&feedcount=10&feedurl=http%3A%2F%2Ffinance.yahoo.com%2Frss%2Fheadline%3Fs%3Dyhoo';

	$result = file_get_contents($query);
	$json_result = json_decode($result);
	$items = $json_result->value->items;

	$feeds = [];
	foreach ($items as $item) {
		$feeds[] = $item->title; 
	}
	echo json_encode($feeds);
}

if (isset($_GET['category']))
	$category = $_GET["category"];
else
	$category = "";

switch ($category) {
	case "topStories":
    // top stories
	$query = 'http://api.nytimes.com/svc/topstories/v1/home.json?api-key=31f908317e61cdf63f264bb88b75881c:14:72395557';
	break;

	case "mostPopular":
    // most popular
	$query = 'http://api.nytimes.com/svc/mostpopular/v2/mostviewed/all-sections/1.json?api-key=c56c29e61017b4bc452a26d1d20a7f95:13:72395557';
	if (isset($_GET['page']))
		$page = intval($_GET['page']);
	else
		$page = 1;
	$page = ($page-1)*20;
	$page = (string) $page;
	$query .= '&offset=' . $page;
	break;

	case "twitter":
    // twiiter
	include_once('twitter.php');
	return;

	case "finance":
	//finance
	financeAPI();
	return;

	default:
	// top stories
	$query = 'http://api.nytimes.com/svc/topstories/v1/home.json?api-key=31f908317e61cdf63f264bb88b75881c:14:72395557';
	break;

}
$result = file_get_contents($query);

$json_result = json_decode($result);
$results = $json_result->results;

$feeds = [];
foreach ($results as $result) {
	$feeds[] = $result->title;
}

echo json_encode($feeds);

?>