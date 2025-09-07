$(document).ready(() => {
	createTemperatureChart();
	createHumidityChart();
	createPressureChart();
	createWindSpeedChart();
	createWindDirectionChart();
	connectWsSensors();
});

let wsSensors;
let temperatureChart;
let humidityChart;
let pressureChart;
let windSpeedChart;
let windDirectionChart;

function connectWsSensors() {
	let deferred = new $.Deferred();

	infoMessage("Socket connecting");

	wsSensors = new WebSocket(`ws://${window.location.host}/sensors.ws`);
	wsSensors.onopen = (evt) => {
		deferred.resolve(wsSensors);
		successMessage("Socket opened").then(() => clearMessage());
	};

	wsSensors.onclose = (evt) => {
		if (evt.wasClean) {
			warningMessage("Socket closed");
		}
		else {
			errorMessage("Socket error");
		}
		setTimeout(() => connectWsSensors(), 10000);
	};

	wsSensors.onerror = (evt) => {
		// NOTHING
	};

	wsSensors.onmessage = (evt) => {
		let sensors = JSON.parse(evt.data);
		updateValues(sensors);
		updateCharts(sensors);
	};

	return deferred.promise();
}

function updateValues(sensors) {
	if (!sensors.temperature || !sensors.humidity || !sensors.pressure) {
		$("#temperature").prop("class", "error").text("ERROR");
		$("#humidity").prop("class", "error").text("ERROR");
		$("#pressure").prop("class", "error").text("ERROR");
	}
	else {
		$("#temperature").removeProp("class").text(sensors.temperature.toFixed(2));
		$("#humidity").removeProp("class").text(sensors.humidity.toFixed(2));
		$("#pressure").removeProp("class").text(sensors.pressure.toFixed(2));
	}

	$("#wind_speed").removeProp("class").text(sensors.wind_speed.toFixed(2));
	$("#wind_direction").removeProp("class").text(sensors.wind_direction);
	$("#rain_intensity").removeProp("class").text(sensors.rain_intensity);
}

function updateCharts(sensors) {
	updateTemperatureChart(sensors.temperature);
	updateHumidityChart(sensors.humidity);
	updatePressureChart(sensors.pressure);
	updateWindSpeedChart(sensors.wind_speed);
	updateWindDirectionChart(sensors.wind_direction);
}

function updateTemperatureChart(temperature) {
	temperatureChart.data.labels.push(Date.now());
	if (temperatureChart.data.labels.length > 100) {
		temperatureChart.data.labels.shift();
	}

	temperatureChart.data.datasets[0].data.push(temperature);

	temperatureChart.data.datasets.forEach((dataset) => {
		if (dataset.data.length > 100) {
			dataset.data.shift();
		}
	});

	temperatureChart.update();
}

function updateHumidityChart(humidity) {
	humidityChart.data.labels.push(Date.now());
	if (humidityChart.data.labels.length > 100) {
		humidityChart.data.labels.shift();
	}

	humidityChart.data.datasets[0].data.push(humidity);

	humidityChart.data.datasets.forEach((dataset) => {
		if (dataset.data.length > 100) {
			dataset.data.shift();
		}
	});

	humidityChart.update();
}

function updatePressureChart(pressure) {
	pressureChart.data.labels.push(Date.now());
	if (pressureChart.data.labels.length > 100) {
		pressureChart.data.labels.shift();
	}

	pressureChart.data.datasets[0].data.push(pressure);

	pressureChart.data.datasets.forEach((dataset) => {
		if (dataset.data.length > 100) {
			dataset.data.shift();
		}
	});

	pressureChart.update();
}

function updateWindSpeedChart(windSpeed) {
	windSpeedChart.data.labels.push(Date.now());
	if (windSpeedChart.data.labels.length > 100) {
		windSpeedChart.data.labels.shift();
	}

	windSpeedChart.data.datasets[0].data.push(windSpeed);

	windSpeedChart.data.datasets.forEach((dataset) => {
		if (dataset.data.length > 100) {
			dataset.data.shift();
		}
	});

	windSpeedChart.update();
}

function directionPTtoAngle(dirPT) {
	switch (dirPT.toLowerCase()) {
		case 'norte': return 0;
		case 'nordeste': return 45;
		case 'leste': return 90;
		case 'sudeste': return 135;
		case 'sul': return 180;
		case 'sudoeste': return 225;
		case 'oeste': return 270;
		case 'noroeste': return 315;
		default: return 0; // fallback
	}
}

const directionsPT = ['Norte', 'Nordeste', 'Leste', 'Sudeste', 'Sul', 'Sudoeste', 'Oeste', 'Noroeste'];

function updateWindDirectionChart(windDirection) {
	const angle = directionPTtoAngle(windDirection); // 0–360°

	// Each segment is 360° / 8 = 45°
	const segmentIndex = Math.round(angle / 45) % 8;

	// Build data array: 1 for selected segment, 0 for the rest
	const dataValues = Array.from({ length: 8 }, (_, i) => i === segmentIndex ? 1 : 0);

	// Update the PolarArea chart
	windDirectionChart.data.datasets[0].data = dataValues;
	windDirectionChart.update();
}

function createTemperatureChart() {
	let ctx = document.getElementById('temperature_chart').getContext('2d');
	temperatureChart = new Chart(ctx,
		{
			type: 'line',
			data:
			{
				datasets: [
					{
						pointBackgroundColor: 'red',
						borderColor: 'red'
					}]
			},
			options:
			{
				responsive: false,
				tooltips:
				{
					enabled: false
				},
				hover:
				{
					mode: null
				},
				spanGaps: true,
				scales:
				{
					yAxes: [
						{
							ticks:
							{
								suggestedMin: 0,
								suggestedMax: 40
							}
						}],
					xAxes: [
						{
							gridLines:
							{
								drawOnChartArea: false
							},
							ticks:
							{
								display: false
							}
						}]
				},
				elements:
				{
					line:
					{
						fill: false
					},
					point:
					{
						radius: 1
					}
				},
				legend: {
					display: false
				}
			}
		});
}
function createHumidityChart() {
	let ctx = document.getElementById('humidity_chart').getContext('2d');
	humidityChart = new Chart(ctx,
		{
			type: 'line',
			data:
			{
				datasets: [
					{
						pointBackgroundColor: 'green',
						borderColor: 'green'
					}]
			},
			options:
			{
				responsive: false,
				tooltips:
				{
					enabled: false
				},
				hover:
				{
					mode: null
				},
				spanGaps: true,
				scales:
				{
					yAxes: [
						{
							ticks:
							{
								suggestedMin: 0,
								suggestedMax: 100
							}
						}],
					xAxes: [
						{
							gridLines:
							{
								drawOnChartArea: false
							},
							ticks:
							{
								display: false
							}
						}]
				},
				elements:
				{
					line:
					{
						fill: false
					},
					point:
					{
						radius: 1
					}
				},
				legend: {
					display: false
				}
			}
		});
}

function createPressureChart() {
	let ctx = document.getElementById('pressure_chart').getContext('2d');
	pressureChart = new Chart(ctx,
		{
			type: 'line',
			data:
			{
				datasets: [
					{
						pointBackgroundColor: 'blue',
						borderColor: 'blue'
					}]
			},
			options:
			{
				responsive: false,
				tooltips:
				{
					enabled: false
				},
				hover:
				{
					mode: null
				},
				spanGaps: true,
				scales:
				{
					yAxes: [
						{
							ticks:
							{
								suggestedMin: 800,
								suggestedMax: 1100
							}
						}],
					xAxes: [
						{
							gridLines:
							{
								drawOnChartArea: false
							},
							ticks:
							{
								display: false
							}
						}]
				},
				elements:
				{
					line:
					{
						fill: false
					},
					point:
					{
						radius: 1
					}
				},
				legend: {
					display: false
				}
			}
		});
}

function createWindSpeedChart() {
	let ctx = document.getElementById('wind_speed_chart').getContext('2d');
	windSpeedChart = new Chart(ctx,
		{
			type: 'line',
			data:
			{
				datasets: [
					{
						pointBackgroundColor: 'purple',
						borderColor: 'purple'
					}]
			},
			options:
			{
				responsive: false,
				tooltips:
				{
					enabled: false
				},
				hover:
				{
					mode: null
				},
				spanGaps: true,
				scales:
				{
					yAxes: [
						{
							ticks:
							{
								suggestedMin: 0,
								suggestedMax: 30
							}
						}],
					xAxes: [
						{
							gridLines:
							{
								drawOnChartArea: false
							},
							ticks:
							{
								display: false
							}
						}]
				},
				elements:
				{
					line:
					{
						fill: false
					},
					point:
					{
						radius: 1
					}
				},
				legend: {
					display: false
				}
			}
		});
}

function createWindDirectionChart() {
	let ctx = document.getElementById('wind_direction_chart').getContext('2d');
	windDirectionChart = new Chart(ctx, {
		type: 'polarArea',
		data: {
			labels: ['N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW'], // 8 compass labels
			datasets: [{
				backgroundColor: [
					'rgba(255,0,0,0.5)',
					'rgba(255,165,0,0.5)',
					'rgba(255,255,0,0.5)',
					'rgba(0,128,0,0.5)',
					'rgba(0,0,255,0.5)',
					'rgba(75,0,130,0.5)',
					'rgba(238,130,238,0.5)',
					'rgba(128,128,128,0.5)'
				]
			}]
		},
		options: {

			responsive: false,
			startAngle: -112.5 * Math.PI / 180,
			scale: {
				ticks: {
					display: false,
					grid: { drawTicks: false },  // no small tick marks
				},
				pointLabels: {
					display: true,
					centerPointLabels: true,
					font: {
						size: 24
					}
				}
			},
			legend: {
				display: false
			}
		}
	});
}

function clearMessage() {
	if (typeof this.fadeOutHandle != "undefined") {
		clearTimeout(this.fadeOutHandle);
	}
	this.fadeOutHandle = setTimeout(() => $("#message").fadeOut(250), 2000);
}

function infoMessage(text) {
	return $("#message").prop("class", "info").text(text).fadeTo(250, 1.0).promise();
}

function successMessage(text) {
	return $("#message").prop("class", "success").text(text).fadeTo(250, 1.0).promise();
}

function warningMessage(text) {
	return $("#message").prop("class", "warning").text(text).fadeTo(250, 1.0).promise();
}

function errorMessage(text) {
	return $("#message").prop("class", "error").text(text).fadeTo(250, 1.0).promise();
}