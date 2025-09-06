$(document).ready(() => {
    updateInfos(true);
    setInterval(updateInfos, 1000);
});

function updateInfos(first) {
    first ? infoMessage("Loading") : null;
    getInfos()
        //.then((info) => drawGraphs(info))
        .then(() => first ? successMessage("Done") : null)
        .then(() => clearMessage())
        .fail((error) => errorMessage(error));
}

//function drawGraphs(info) {
//    for (const [i, sensor] of info.sensors.entries()) {
//        var canvas = document.getElementById(`sensor_graph_${i}`);
//        var ctx = canvas.getContext("2d");
//        ctx.canvas.width = canvas.offsetWidth;
//        ctx.canvas.height = canvas.offsetHeight;
//        ctx.fillStyle = "blue";
//        ctx.fillRect(0, 0, canvas.offsetWidth * (Math.min(Math.max(sensor.percent, 0.0), 100.0) / 100.0), canvas.offsetHeight);
//    };
//}

function getInfos() {
    var deferred = new $.Deferred();

    $.ajax({
        type: "GET",
        url: "/infos.json",
        accepts: 'application/json',
        timeout: 5000
    })
        .done((info) => {
            $("#values tbody tr").remove();

            if (!info.temperature || !info.humidity || !info.pressure) {
                $("#temperature").prop("class", "error").text("ERROR");
                $("#humidity").prop("class", "error").text("ERROR");
                $("#pressure").prop("class", "error").text("ERROR");
            }
            else {
                $("#temperature").removeProp("class").text(info.temperature);
                $("#humidity").removeProp("class").text(info.humidity);
                $("#pressure").removeProp("class").text(info.pressure);
            }

            $("#wind_speed").removeProp("class").text(info.wind_speed);
            $("#wind_direction").removeProp("class").text(info.wind_direction);
            $("#rain_intensity").removeProp("class").text(info.rain_intensity);

            deferred.resolve(info);
        })
        .fail((xhr, status, error) => {
            deferred.reject(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
        });
    return deferred.promise();
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