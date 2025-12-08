$(document).ready(() => {
    createTemperatureChart();
	createHumidityChart();
	createPressureChart();
	createWindSpeedChart();
    handleFilter();
    getDateTime().then(() => loadCSV()).then(() => clearMessage());
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
                loadCSV().then(() => clearMessage());
            }
            else if ($("#filter").prop("action").endsWith("download")) {
                handleDownload();
            }
        }
    });
}

function handleDownload() {
    const params = new URLSearchParams({
        start: `${$("#filter_start_date").val()} ${$("#filter_start_time").val()}`,
        end: `${$("#filter_end_date").val()} ${$("#filter_end_time").val()}`,
    })
    window.location = `/data.csv?${params}`;
}

function getDateTime() {
    var deferred = new $.Deferred();
    $.ajax({
        type: "GET",
        url: `http://${window.location.host || "192.168.1.200"}/datetime.json`,
        accepts: 'application/json',
        timeout: 5000,
        beforeSend: () => {
            $("#filter :input").prop("disabled", true);
            infoMessage("Loading");
        }
    })
	.done((dateTime) => {
		var [date, time] = dateTime.split(" ");
		$("#filter_start_date").val(date);
		$("#filter_start_time").val("00:00:00");
		$("#filter_end_date").val(date);
		$("#filter_end_time").val("23:59:59");

		successMessage("Data hora carregada");
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

async function loadCSV() {

    $("#filter :input").prop("disabled", true);
    $("#result tbody tr").remove();
    clearCharts();

    infoMessage("Carregando dados");

    try {
        const params = new URLSearchParams({
			start: `${$("#filter_start_date").val()} ${$("#filter_start_time").val()}`,
			end: `${$("#filter_end_date").val()} ${$("#filter_end_time").val()}`,
		});
		
		const response = await fetch(`http://${window.location.host || "192.168.1.200"}/data.csv?${params}`);
		const text = await response.text();
		
		let template = $($.parseHTML($("#data_template").html()));
		let newRows = [];
		
		for (const line of text.split("\n")) {
			if (!line.trim()) continue;
		
			const cells = line.split(";");
		
			// skip header
			if (cells[0].toLowerCase() === "datahora") continue;
		
			let row = template.clone();
		
			const data = {
				datetime: new Date(cells[0]),
				temperature: parseFloat(cells[1].replace(',', '.')),
				humidity: parseFloat(cells[2].replace(',', '.')),
				pressure: parseFloat(cells[3].replace(',', '.')),
				wind_speed: parseFloat(cells[4].replace(',', '.')),
				wind_direction: cells[5],
				rain_intensity: cells[6],
			};
		
			row.find("#data_date").text(data.datetime.toLocaleDateString());
			row.find("#data_time").text(data.datetime.toLocaleTimeString());
			row.find("#data_temperature").text(data.temperature.toFixed(2));
			row.find("#data_humidity").text(data.humidity.toFixed(2));
			row.find("#data_pressure").text(data.pressure.toFixed(2));
			row.find("#data_wind_speed").text(data.wind_speed.toFixed(2));
			row.find("#data_wind_direction").text(data.wind_direction);
			row.find("#data_rain_intensity").text(data.rain_intensity);
		
			newRows.push(row);
		
			updateCharts(data);
		}
		
		$("#result tbody").append(newRows);

        //// último pedaço
        //if (buffer.trim()) {
        //    const cells = buffer.split(";");
        //    if (cells[0].toLowerCase() !== "datahora") {
        //        const [date, time] = cells[0].split(" ");
        //        const row = $("<tr>");
        //        row.append($("<td>").text(date || ""));
        //        row.append($("<td>").text(time || ""));
        //        row.append($("<td>").text(parseFloat(cells[1]).toFixed(2)));
        //        row.append($("<td>").text(parseFloat(cells[2]).toFixed(2)));
        //        row.append($("<td>").text(parseFloat(cells[3]).toFixed(2)));
        //        row.append($("<td>").text(parseFloat(cells[4]).toFixed(2)));
        //        row.append($("<td>").text(cells[5] || ""));
        //        row.append($("<td>").text(cells[6] || ""));
        //        row.append("</tr>");
        //        row.appendTo($("#result tbody"));
        //    }
        //}

        successMessage("Dados carregados");

    } catch (err) {
        errorMessage(`Erro: ${err}`);
    }
    finally {
        $("#filter :input").prop("disabled", false);
    }
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
						borderColor: 'red',
                        tension: 0.4,
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
						borderColor: 'green',
                        tension: 0.4,
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
						borderColor: 'blue',
                        tension: 0.4,
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
								suggestedMin: 915,
								suggestedMax: 935
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
						borderColor: 'purple',
                        tension: 0.4,
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
								suggestedMax: 20
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

    temperatureChart.data.labels.push(data.datetime);
    temperatureChart.data.datasets[0].data.push(data.temperature);
    temperatureChart.update();

    humidityChart.data.labels.push(data.datetime);
    humidityChart.data.datasets[0].data.push(data.humidity);
    humidityChart.update();

    pressureChart.data.labels.push(data.datetime);
    pressureChart.data.datasets[0].data.push(data.pressure);
    pressureChart.update();

    windSpeedChart.data.labels.push(data.datetime);
    windSpeedChart.data.datasets[0].data.push(data.wind_speed);
    windSpeedChart.update();

	//updateWindDirectionChart(sensors.wind_direction);
}

function clearCharts(){
    temperatureChart.data.labels = [];
    temperatureChart.data.datasets[0].data = [];
    temperatureChart.update();

    humidityChart.data.labels = [];
    humidityChart.data.datasets[0].data = [];
    humidityChart.update();

    pressureChart.data.labels = [];
    pressureChart.data.datasets[0].data = [];
    pressureChart.update();

    windSpeedChart.data.labels = [];
    windSpeedChart.data.datasets[0].data = [];
    windSpeedChart.update();
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