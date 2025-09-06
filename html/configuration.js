$(document).ready(() => {
    handleConfiguration();

    getDateTime().then(() => getConfiguration()).then(() => clearMessage());
});

function handleConfiguration() {
    $("#wind_speed").submit((event) => {
        event.preventDefault();
        if ($("#wind_speed")[0].checkValidity()) {
            setWindSpeed().then(() => clearMessage());
        }
    });

    $("#datetime").submit((event) => {
        event.preventDefault();
        if ($("#datetime")[0].checkValidity()) {
            setDateTime().then(() => clearMessage());
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
            $("input,select").prop("disabled", true);
            infoMessage("Sending");
        }
    })
        .done((dateTime) => {
            successMessage(msg ?? "Done");
            deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            $("input,select").prop("disabled", false);
        });
    return deferred.promise();
}

function setSensors() {
    var cfg = {
        sensors: []
    };

    $("#sensors tbody tr").each((i, s) => {
        cfg.sensors.push({
            enabled: $(`#sensor_enabled_${i}`).prop("checked"),
            name: $(`#sensor_name_${i}`).prop("value"),
            type: parseInt($(`#sensor_type_${i}`).prop("value")),
            min: parseFloat($(`#sensor_min_${i}`).prop("value")),
            max: parseFloat($(`#sensor_max_${i}`).prop("value")),
            calibration: {
                factor: parseFloat($(`#sensor_calibration_radius_${i}`).prop("value")),
                offset: parseFloat($(`#sensor_calibration_linear_coefficient_${i}`).prop("value"))
            },
            alarm: {
                enabled: $(`#sensor_alarm_enabled_${i}`).prop("checked"),
                value: parseFloat($(`#sensor_alarm_value_${i}`).prop("value"))
            }
        });
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
            $("input,select").prop("disabled", true);
            infoMessage("Sending");
        }
    })
        .done((msg) => {
            successMessage(msg ?? "Done");
            deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            $("input,select").prop("disabled", false);
        });
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
            $("input,select").prop("disabled", true);
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

            var template = $($.parseHTML($("#sensor_template").html()));
            for (const [i, s] of cfg.sensors.entries()) {
                var row = template.clone();
                row.find("#sensor_number").text(i + 1);
                row.find("#sensor_enabled").prop("checked", s.enabled);
                row.find("#sensor_name").prop("value", s.name);
                row.find("#sensor_type").prop("value", s.type);
                row.find("#sensor_min").prop("value", s.min);
                row.find("#sensor_max").prop("value", s.max);
                row.find("#sensor_calibration_radius").prop("value", s.calibration.radius);
                row.find("#sensor_calibration_linear_coefficient").prop("value", s.calibration.linear_coefficient);
                row.find("#sensor_alarm_enabled").prop("checked", s.alarm.enabled);
                row.find("#sensor_alarm_value").prop("value", s.alarm.value);
                for (var c of row.find("*")) {
                    if (c.id) {
                        c.id += `_${i}`;
                    }
                    if (c.htmlFor) {
                        c.htmlFor += `_${i}`;
                    }
                }
                row.appendTo($("#sensors table tbody"));
            }
            successMessage("Done");
            deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            $("input,select").prop("disabled", false);
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

            successMessage("Done");
            deferred.resolve(current);
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            $("input,select").prop("disabled", false);
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