$(document).ready(() => {
    handleFiles();
    handleConfiguration();

    getDateTime().then(() => getConfiguration()).then(() => clearMessage());
});

function handleFiles() {
    $("#firmware").submit((event) => {
        event.preventDefault();
        if ($("#firmware")[0].checkValidity()) {
            uploadFirmware();
        }
    });

    $("#configuration").submit((event) => {
        event.preventDefault();
        let submitter = event.originalEvent.submitter.id;
        if (submitter == "configuration_upload") {
            if ($("#configuration")[0].checkValidity()) {
                uploadConfiguration();
            }
        }
        else if (submitter == "configuration_download") {
            window.open("configuration.json");
        }
    });
}

function handleConfiguration() {

    $("#datetime").submit((event) => {
        event.preventDefault();
        if ($("#datetime")[0].checkValidity()) {
            setDateTime().then(() => clearMessage());
        }
    });

    $("#wind_speed").submit((event) => {
        event.preventDefault();
        if ($("#wind_speed")[0].checkValidity()) {
            setWindSpeed().then(() => clearMessage());
        }
    });

    $("#wind_direction").submit((event) => {
        event.preventDefault();
        if ($("#wind_direction")[0].checkValidity()) {
            setWindDirection().then(() => clearMessage());
        }
    });

    $("#access_point").submit((event) => {
        event.preventDefault();
        if ($("#access_point")[0].checkValidity()) {
            setAccessPoint().then(() => clearMessage());
        }
    });

    $("#station").submit((event) => {
        event.preventDefault();
        if ($("#station")[0].checkValidity()) {
            setStation().then(() => clearMessage());
        }
    });
}

function setDateTime() {
    var deferred = new $.Deferred();

    var newDateTime = `${$("#datetime_new_date").val()} ${$("#datetime_new_time").val()}`;

    $.ajax({
        type: "POST",
        url: "/datetime.json",
        contentType: 'application/json',
        timeout: 5000,
        data: JSON.stringify(newDateTime),
        beforeSend: () => {
            disableInput();
            infoMessage("Sending");
        }
    })
        .done((msg) => {
            successMessage(msg ?? "Datetime saved").then(() => reload());
            //deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            enableInput();
            deferred.reject();
        })
        //.always(() => {
        //    enableInput();
        //});
    return deferred.promise();
}

function setWindSpeed() {
    var cfg = {
        wind_speed: {
            radius: parseFloat($("#wind_speed_radius").prop("value"))
        }
    };
    return setConfiguration(cfg);
}

function setWindDirection() {
    var cfg = {
        wind_direction: {
            threshoulds: {}
        }
    };

    $("#wind_direction tbody tr").each((i, s) => {
        var name = $(`#wind_direction_name_${i}`).text();
        cfg.wind_direction.threshoulds[name] = {
            min: parseFloat($(`#wind_direction_min_${i}`).prop("value")),
            max: parseFloat($(`#wind_direction_max_${i}`).prop("value")),
        }
    });

    return setConfiguration(cfg);
}

function setRainIntensity() {
    var cfg = {
        rain_intensity: {
            threshoulds: {}
        }
    };

    $("#rain_intensity tbody tr").each((i, s) => {
        var name = $(`#rain_intensity_name_${i}`).text();
        cfg.rain_intensity.threshoulds[name] = {
            min: parseFloat($(`#rain_intensity_min_${i}`).prop("value")),
            max: parseFloat($(`#rain_intensity_max_${i}`).prop("value")),
        }
    });

    return setConfiguration(cfg);
}

function setAccessPoint() {
    var cfg = {
        access_point: {
            enabled: $("#access_point_enabled").prop("checked"),
            mac: $("#access_point_mac").prop("value").split("-").map((s) => parseInt(s, 16)),
            ip: $("#access_point_ip").prop("value").split(".").map((s) => parseInt(s, 10)),
            netmask: $("#access_point_netmask").prop("value").split(".").map((s) => parseInt(s, 10)),
            gateway: $("#access_point_gateway").prop("value").split(".").map((s) => parseInt(s, 10)),
            port: parseInt($("#access_point_port").prop("value"), 10),
            user: $("#access_point_user").prop("value"),
            password: $("#access_point_password").prop("value"),
            duration: parseInt($("#access_point_duration").prop("value"), 10)
        }
    };
    return setConfiguration(cfg);
}

function setStation() {
    var cfg = {
        station: {
            enabled: $("#station_enabled").prop("checked"),
            mac: $("#station_mac").prop("value").split("-").map((s) => parseInt(s, 16)),
            ip: $("#station_ip").prop("value").split(".").map((s) => parseInt(s, 10)),
            netmask: $("#station_netmask").prop("value").split(".").map((s) => parseInt(s, 10)),
            gateway: $("#station_gateway").prop("value").split(".").map((s) => parseInt(s, 10)),
            port: parseInt($("#station_port").prop("value"), 10),
            user: $("#station_user").prop("value"),
            password: $("#station_password").prop("value")
        }
    };
    return setConfiguration(cfg);
}

function setConfiguration(cfg) {
    var deferred = new $.Deferred();

    $.ajax({
        type: "POST",
        url: "/configuration.json",
        contentType: 'application/json',
        timeout: 5000,
        data: JSON.stringify(cfg),
        beforeSend: () => {
            disableInput();
            infoMessage("Sending");
        }
    })
        .done((msg) => {
            successMessage(msg ?? "Configuration saved").then(() => reload());
            //deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            enableInput();
            deferred.reject();
        })
        //.always(() => {
        //    enableInput();
        //});
    return deferred.promise();
}

function getConfiguration() {
    var deferred = new $.Deferred();
    $.ajax({
        type: "GET",
        url: "/configuration.json",
        accepts: 'application/json',
        timeout: 5000,
        beforeSend: () => {
            disableInput();
            infoMessage("Loading");
        }
    })
        .done((cfg) => {
            $("#access_point_enabled").prop("checked", cfg.access_point.enabled);
            $("#access_point_mac").prop("value", cfg.access_point.mac.map((n) => n.toString(16).toUpperCase().padStart(2, "0")).join("-"));
            $("#access_point_ip").prop("value", cfg.access_point.ip.map((n) => n.toString(10)).join("."));
            $("#access_point_netmask").prop("value", cfg.access_point.netmask.map((n) => n.toString(10)).join("."));
            $("#access_point_gateway").prop("value", cfg.access_point.gateway.map((n) => n.toString(10)).join("."));
            $("#access_point_port").prop("value", cfg.access_point.port);
            $("#access_point_user").prop("value", cfg.access_point.user);
            $("#access_point_password").prop("value", cfg.access_point.password);
            $("#access_point_duration").prop("value", cfg.access_point.duration);

            $("#station_enabled").prop("checked", cfg.station.enabled);
            $("#station_mac").prop("value", cfg.station.mac.map((n) => n.toString(16).toUpperCase().padStart(2, "0")).join("-"));
            $("#station_ip").prop("value", cfg.station.ip.map((n) => n.toString(10)).join("."));
            $("#station_netmask").prop("value", cfg.station.netmask.map((n) => n.toString(10)).join("."));
            $("#station_gateway").prop("value", cfg.station.gateway.map((n) => n.toString(10)).join("."));
            $("#station_port").prop("value", cfg.station.port);
            $("#station_user").prop("value", cfg.station.user);
            $("#station_password").prop("value", cfg.station.password);

            $("#wind_speed_radius").prop("value", cfg.wind_speed.radius.toFixed(2));

            {
                var template = $($.parseHTML($("#wind_direction_template").html()));
                for (const [i, s] of Object.entries(cfg.wind_direction.threshoulds).entries()) {
                    var row = template.clone();
                    row.find("#wind_direction_name").text(s[0]);
                    row.find("#wind_direction_min").prop("value", s[1].min);
                    row.find("#wind_direction_max").prop("value", s[1].max);
                    for (var c of row.find("*")) {
                        if (c.id) {
                            c.id += `_${i}`;
                        }
                        if (c.htmlFor) {
                            c.htmlFor += `_${i}`;
                        }
                    }
                    row.appendTo($("#wind_direction table tbody"));
                }
            }

            {
                var template = $($.parseHTML($("#rain_intensity_template").html()));
                for (const [i, s] of Object.entries(cfg.rain_intensity.threshoulds).entries()) {
                    var row = template.clone();
                    row.find("#rain_intensity_name").text(s[0]);
                    row.find("#rain_intensity_min").prop("value", s[1].min);
                    row.find("#rain_intensity_max").prop("value", s[1].max);
                    for (var c of row.find("*")) {
                        if (c.id) {
                            c.id += `_${i}`;
                        }
                        if (c.htmlFor) {
                            c.htmlFor += `_${i}`;
                        }
                    }
                    row.appendTo($("#rain_intensity table tbody"));
                }
            }

            successMessage("Configuration loaded").then(() => clearMessage());
            deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            enableInput();
        });
    return deferred.promise();
}

function getDateTime() {
    var deferred = new $.Deferred();
    $.ajax({
        type: "GET",
        url: "/datetime.json",
        accepts: 'application/json',
        timeout: 5000,
        beforeSend: () => {
            $("input").prop("disabled", true);
            infoMessage("Loading");
        }
    })
        .done((dateTime) => {
            var [date, time] = dateTime.split(" ");
            $("#datetime_current_date").val(date);
            $("#datetime_current_time").val(time);

            var current = new Date(`${date}T${time}.000Z`);

            clearInterval(this.updateHandle);
            this.updateHandle = setInterval(() => {
                current = new Date(current.getTime() + 1000);
                var [date, time] = current.toISOString().split('T');
                $("#datetime_current_date").val(date.slice(0, 10));
                $("#datetime_current_time").val(time.slice(0, 8));
            }, 1000);

            successMessage("Datetime loaded").then(() => clearMessage());
            deferred.resolve(current);
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            enableInput();
        });
    return deferred.promise();
}

function uploadFirmware() {
    let deferred = new $.Deferred();

    let file = $("#firmware_file")[0].files[0];
    if (file.size > 1945600) {
        errorMessage("File size must be 1945600 bytes or less");
        deferred.reject();
    }
    else {
        let formData = new FormData();
        formData.append("firmware.bin", file, file.name);

        $.ajax(
            {
                type: "POST",
                url: "/firmware.bin",
                contentType: false,
                processData: false,
                data: formData,
                timeout: 300000,
                beforeSend: () => {
                    disableInput();
                    infoMessage("Uploading");
                }
            })
            .done((msg) => {
                successMessage(msg ?? "Firmware uploaded").then(() => reload());
                //deferred.resolve();
            })
            .fail((xhr, status, error) => {
                errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
                deferred.reject();
            });
        //.always(() =>
        //{
        //	enableInput();
        //});
    }

    return deferred.promise();
}

function uploadConfiguration() {
    let deferred = new $.Deferred();

    let file = $("#configuration_file")[0].files[0];
    if (file.size > 4096) {
        errorMessage("File size must be 4096 bytes or less");
        deferred.reject();
    }
    else {
        let formData = new FormData();
        formData.append("configuration.json", file, file.name);

        $.ajax(
            {
                type: "POST",
                url: "/configuration.json",
                contentType: false,
                processData: false,
                data: formData,
                timeout: 30000,
                beforeSend: () => {
                    disableInput();
                    infoMessage("Uploading");
                }
            })
            .done((msg) => {
                successMessage(msg ?? "Configuration uploaded").then(() => reload());
                //deferred.resolve();
            })
            .fail((xhr, status, error) => {
                errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
                deferred.reject();
            });
        //.always(() =>
        //{
        //	enableInput();
        //});
    }

    return deferred.promise();
}

function reload()
{
	setTimeout(() =>
	{
		infoMessage("Reloading page in 15 seconds");
		setTimeout(() => location.reload(), 15000);
	}, 3000);
}

function disableInput() {
    $("input,select,button").prop("disabled", true);
}

function enableInput() {
    $("input,select,button").prop("disabled", false);
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