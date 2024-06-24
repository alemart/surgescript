(function() {
    const script = document.createElement('script');

    script.addEventListener('error', function(e) {
        console.log(`Can't load analytics`, e);
    });

    script.type = 'text/javascript';
    script.async = true;
    script.dataset.goatcounter = 'https://stats.opensurge2d.org/count';
    script.src = 'https://gc.zgo.at/count.js';

    document.head.appendChild(script);
})();
