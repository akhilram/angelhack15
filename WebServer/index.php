<?php

$category = $_GET["category"];
switch ($category) {
	case "topStories":
	// top stories
	$query = 'http://api.nytimes.com/svc/topstories/v1/home.json?api-key=31f908317e61cdf63f264bb88b75881c:14:72395557';
	break;
	
	case "mostPopular":
	// most popular
	$query = 'http://api.nytimes.com/svc/mostpopular/v2/mostviewed/all-sections/1.json?api-key=c56c29e61017b4bc452a26d1d20a7f95:13:72395557';
	break;

	default:
		# code...
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