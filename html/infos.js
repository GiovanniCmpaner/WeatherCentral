$(document).ready(() => {
    connectWsSensors();
});

let wsSensors;

function connectWsSensors()
{
	let deferred = new $.Deferred();

	infoMessage("Socket connecting");

	wsSensors = new WebSocket(`ws://${window.location.host}/sensors.ws`);
	wsSensors.onopen = (evt) =>
	{
		deferred.resolve(wsSensors);
		successMessage("Socket opened").then(() => clearMessage());
	};

	wsSensors.onclose = (evt) =>
	{
		if (evt.wasClean)
		{
			warningMessage("Socket closed");
		}
		else
		{
			errorMessage("Socket error");
		}
		setTimeout(() => connectWsSensors(), 10000);
	};

	wsSensors.onerror = (evt) =>
	{
		// NOTHING
	};

	wsSensors.onmessage = (evt) =>
	{
		let sensors = JSON.parse(evt.data);
		updateValues(sensors);
	};

	return deferred.promise();
}

function updateValues(sensors)
{
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