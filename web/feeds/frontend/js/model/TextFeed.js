/**
 * Created by vrushali on 15/10/15.
 */
define('TextFeed', ["jquery", "Feed"], function ($) {
    var Feed = require('Feed');
    var TextFeed = function (feed_name, feed_path, config) {
        this.base = Feed;
        this.base(feed_name, feed_path, config);
        this.height = this.config.height != undefined ? this.config.height : 0;
    };
    TextFeed.prototype = Object.create(Feed.prototype);
    TextFeed.prototype.load = function () {
        $.ajax({
            crossDomain: true,
            context: this,
            url: this.get_url(),
            timeout: globals.timeout
        }).done(function (data, textStatus, jqXHR) {
            this.onload(data);
        }).fail(function (jqXHR, textStatus, errorThrown) {
            this.onerror();
        });
        if (this.height > 0) {
            this.target.height(this.height);
        }
    };
    TextFeed.prototype.onload_ = function (data) {
        if (data && data.length && data[data.length - 1] == '\n') {
            data = data.substring(0, data.length - 1);
        }
        var orig_data = data;
        data = data ? data : '&nbsp;';

        if (this.form_show_buttons) {
            var panel = $(this.target).find(".text-pre");
            if (panel.length > 0) {
                $(panel).html(data);
            }
            else {
                this.target.append('<tr><td><pre class="text-pre">' + data + '</pre></td></tr>');
                panel = $(this.target).find(".text-pre");
            }
            if (orig_data) {
                $(panel).css("overflow-y", "scroll");
            }
            else {
                $(panel).css("overflow", "auto");
            }
            var this_ = this;
            var height = 100;
            if (this.height > 0) {
                height = this.height;
            }
            $(panel).css("max-height", function () {
                return height;
            });

            this.draw();
            $(panel).scrollTop($(panel)[0].scrollHeight);
        }
        else {
            this.target.append('<tr><td><pre>' + data + '</pre></td></tr>');
            this.draw();
        }
    };
    TextFeed.prototype.draw = function () {
        while (this.target.find('tbody tr').length > this.config.text.show_items) {
            this.target.find('tbody tr').first().remove();
        }
    };
    return TextFeed;
});
