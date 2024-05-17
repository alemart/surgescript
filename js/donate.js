document.addEventListener('DOMContentLoaded', function() {
    const script = document.createElement('script');

    script.addEventListener('load', function() {
        kofiWidgetOverlay.draw('alemart', {
            'type': 'floating-chat',
            'floating-chat.donateButton.text': 'Support me',
            'floating-chat.donateButton.background-color': '#4051b5',
            'floating-chat.donateButton.text-color': 'white'
        });
    });

    script.addEventListener('error', function(e) {
        console.log(`Can't load the Ko-fi widget`, e);
    });

    script.type = 'text/javascript';
    script.async = true;
    script.src = 'https://storage.ko-fi.com/cdn/scripts/overlay-widget.js';

    document.body.appendChild(script);

    const style = document.createElement('style');
    style.appendChild(document.createTextNode(`
        .floatingchat-container-wrap, .floatingchat-container-wrap-mobi,
        .floating-chat-kofi-popup-iframe, .floating-chat-kofi-popup-iframe-mobi {
            right: 16px !important;
            left: initial !important;
        }
    `));
    document.head.appendChild(style);
});
