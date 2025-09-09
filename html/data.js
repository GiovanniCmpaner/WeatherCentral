$(document).ready(() => {
    createTemperatureChart();
	createHumidityChart();
	createPressureChart();
	createWindSpeedChart();
    handleFilter();
    getDateTime();
    getData().then(() => clearMessage());
});

let temperatureChart;
let humidityChart;
let pressureChart;
let windSpeedChart;;

function handleFilter() {
    $("#filter_button").click((event) => {
        $("#filter").prop("action", "filter");
    });
    $("#filter_download").click((event) => {
        $("#filter").prop("action", "download");
    });
    $("#filter").submit((event) => {
        event.preventDefault();
        if ($("#filter")[0].checkValidity()) {
            if ($("#filter").prop("action").endsWith("filter")) {
                getData().then(() => clearMessage());
            }
            else if ($("#filter").prop("action").endsWith("download")) {
                handleDownload();
            }
        }
    });
}

function handleDownload() {
    var filter = buildfilter();
    window.location = `/data.csv?${new URLSearchParams(filter).toString()}`;
}

function getDateTime() {

    var now = new Date().toISOString();
    var [date, time] = now.split("T");
    $("#filter_start_date").val(date);
    $("#filter_start_time").val("00:00:00");
    $("#filter_end_date").val(date);
    $("#filter_end_time").val("23:59:59");
}

function buildfilter() {
    var filter = {};
    if ($("#filter_start_date").val() && $("#filter_start_time").val()) {
        filter.start = `${$("#filter_start_date").val()} ${$("#filter_start_time").val()}`;
    }
    else {
        delete filter.start;
    }
    if ($("#filter_end_date").val() && $("#filter_end_time").val()) {
        filter.end = `${$("#filter_end_date").val()} ${$("#filter_end_time").val()}`;
    }
    else {
        delete filter.end;
    }
    if (Object.values(filter).length > 0) {
        return filter;
    }
    else {
        return null;
    }
}

function getData(filter) {
    var deferred = new $.Deferred();

    $.ajax({
        type: "GET",
        url: `http://${window.location.host || "192.168.1.200"}/data.json`,
        accepts: 'application/json',
        timeout: 5000,
        data: filter,
        beforeSend: () => {
            $("#filter :input").prop("disabled", true);
            infoMessage("Loading");
        }
    })
        .done((data) => {
            $("#result tbody tr").remove();

            let template = $($.parseHTML($("#data_template").html()));
            for (const [i, d] of data.entries()) {
                let row = template.clone();
                let [date, time] = d.datetime.split(" ");
                row.find("#data_date").text(date);
                row.find("#data_time").text(time);
                row.find("#data_temperature").text(d.temperature.toFixed(2));
                row.find("#data_humidity").text(d.humidity.toFixed(2));
                row.find("#data_pressure").text(d.pressure.toFixed(2));
                row.find("#data_wind_speed").text(d.wind_speed.toFixed(2));
                row.find("#data_wind_direction").text(d.wind_direction);
                row.find("#data_rain_intensity").text(d.rain_intensity);
                for (let c of row.find("*")) {
                    if (c.id) {
                        c.id += `_${i}`;
                    }
                    if (c.htmlFor) {
                        c.htmlFor += `_${i}`;
                    }
                }
                row.appendTo($("#result tbody"));
            }

            updateCharts(data);

            successMessage("Dados carregados");
            deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            $("#filter :input").prop("disabled", false);
        });
    return deferred.promise();
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

function updateCharts(data) {

    temperatureChart.data.labels = data.map(d => d.datetime);
    temperatureChart.data.datasets[0].data = data.map(d => d.temperature);
    temperatureChart.update();

    humidityChart.data.labels = data.map(d => d.datetime);
    humidityChart.data.datasets[0].data = data.map(d => d.humidity);
    humidityChart.update();

    pressureChart.data.labels = data.map(d => d.datetime);
    pressureChart.data.datasets[0].data = data.map(d => d.pressure);
    pressureChart.update();

    windSpeedChart.data.labels = data.map(d => d.datetime);
    windSpeedChart.data.datasets[0].data = data.map(d => d.wind_speed);
    windSpeedChart.update();

	//updateWindDirectionChart(sensors.wind_direction);
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