//if (typeof Drupal !== 'undefined' && typeof jQuery !== 'undefined') {
  // Only load if Drupal and jQuery are defined.
  (function ($) {
    /**
     * Get network speed in Mbps.
     */
    function picture_get_network_speed(callback) {
      if ($.cookie('picture_network_speed') == null) {
        var image_url = Drupal.settings.picture.network_speed_test_image; // URL
        var image_size = Drupal.settings.picture.network_speed_test_image_size; // bytes.

        var startTime, endTime;
        var download = new Image();
        download.onload = function () {
          endTime = (new Date()).getTime();
          var duration = (endTime - startTime) / 1000;
          var bitsLoaded = image_size * 8;
          var speedBps = (bitsLoaded / duration).toFixed(2);
          var speedMbps = (speedBps / (1024 * 1024)).toFixed(2);
          $.cookie('picture_network_speed', speedMbps, {
            expires: 1,
            path: Drupal.settings.basePath
          });
          callback(speedMbps);
        }

        download.onerror = function (err, msg) {
          console.log("Invalid image, or error downloading");
          callback(null);
        }

        startTime = (new Date()).getTime();
        var cacheBuster = "?nnn=" + startTime;
        download.src = image_url + cacheBuster;
      }
      callback($.cookie('picture_network_speed'));
    }
    Drupal.behaviors.picture = {
      attach: function (context) {
        // Don't load if there's native picture element support.
        if (!('HTMLPictureElement' in window)) {
          // Ensure we always pass a raw DOM element to picture fill, otherwise it
          // will fallback to the document scope and maybe handle to much.
          var imgs = $(context).find('img');
          if (imgs.length) {
            window.picturefill({
              elements: imgs.get()
            });
          }
        }
        // If this is an opened colorbox ensure the content dimensions are set
        // properly. colorbox.js of the colorbox modules sets #cboxLoadedContent
        // as context.
        if (context === '#cboxLoadedContent' && $(context).find('picture').length) {
          // Try to resize right away.
          $.colorbox.resize();
          // Make sure the colorbox resizes always when the image is changed.
          $('img', context).once('colorbox-lazy-load', function(){
            $(this).load(function(){
              // Ensure there's no max-width / max-height otherwise we won't get
              // the proper values. We could use naturalWeight / naturalHeight
              // but that's not supported by <IE9 and Opera.
              this.style.maxHeight = $(window).height() + 'px';
              this.style.maxWidth = $(window).width() + 'px';
              $.colorbox.resize({innerHeight: this.height, innerWidth: this.width});
              // Remove overwrite of this values again to ensure we respect the
              // stylesheet.
              this.style.maxHeight = null;
              this.style.maxWidth = null;
            });
          });
        }
        picture_get_network_speed(function(network_speed){
          console.log(network_speed);
          $('picture source').each(function(){
            $source = $(this);
            var attr = $source.attr('data-' + window.devicePixelRatio + 'x');

            // For some browsers, `attr` is undefined; for others, `attr` is false. Check for both.
            if (typeof attr !== typeof undefined && attr !== false) {
              // Data attribute will be holding image src, network speed lower limit and upper limit.
              // All three values seperated with commas (,).
              var parts = attr.split(',');
              var src = parts[0],
              speed_start = parseFloat(parts[1]),
              speed_end = parseFloat(parts[2]);
              if (speed_start <= network_speed && network_speed <= speed_end) {
                $source.attr('srcset', $source.attr('srcset') + ', ' + src + ' ' +  window.devicePixelRatio + 'x');
                $source.removeAttr('data-' + window.devicePixelRatio + 'x');
              }
            }
          });
        })
      }
    };
  })(jQuery);
// }
