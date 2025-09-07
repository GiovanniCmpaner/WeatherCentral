$(document).ready(() => {
    handleFilter();
    getDateTime().done(() => handleLoadMore());
});

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
                handleLoadMore();
            }
            else if ($("#filter").prop("action").endsWith("download")) {
                handleDownload();
            }
        }
    });
}

function handleDownload() {
    var filter = buildfilter();
    if (filter) {
        window.location = `/data.csv?${new URLSearchParams(filter).toString()}`;
    }
    else {
        window.location = `/data.csv`;
    }
}

function handleLoadMore(withoutFilter) {
    $(window).unbind("scroll");
    clearTimeout(this.updateHandle);

    getData(withoutFilter ? null : buildfilter())
        .done((count) => {
            if (count == 20) {
                $(window).scroll(() => {
                    if ($(window).height() + $(window).scrollTop() > $("body").height() * 0.75) {
                        $(window).unbind("scroll");
                        handleLoadMore(true);
                    }
                });
            }
            else {
                this.updateHandle = setTimeout(() => handleLoadMore(true), 30000);
            }
        })
        .then(clearMessage);
}

function getDateTime() {
    var deferred = new $.Deferred();
    $.ajax({
        type: "GET",
        url: "/datetime.json",
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

            successMessage("Done");
            deferred.resolve(new Date(dateTime));
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

    var clear = true;
    if (filter == null) {
        clear = false;
        if (typeof this.prevFilter != "undefined") {
            filter = this.prevFilter;
        }
    }

    $.ajax({
        type: "GET",
        url: "/data.json",
        accepts: 'application/json',
        timeout: 5000,
        data: filter,
        beforeSend: () => {
            $("#filter :input").prop("disabled", true);
            infoMessage("Loading");
        }
    })
        .done((data) => {
            if (clear) {
                $("#result tbody tr").remove();
            }

            let lastDateTime = null;

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
                lastDateTime = Math.max(lastDateTime, d.id);
            }

            this.prevFilter = filter;
            if (lastDateTime != null) {
                if (this.prevFilter != null) {
                    this.prevFilter.start = lastDateTime + 1;
                }
                else {
                    this.prevFilter = { id: lastDateTime + 1 };
                }
            }

            successMessage("Dados");
            deferred.resolve(data.length);
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