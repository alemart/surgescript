document.addEventListener('DOMContentLoaded', () => {
    const script = document.createElement('script');

    script.addEventListener('load', () => {
        kofiWidgetOverlay.draw('alemart', {
            'type': 'floating-chat',
            'floating-chat.donateButton.text': 'Support me',
            'floating-chat.donateButton.background-color': '#4051b5',
            'floating-chat.donateButton.text-color': 'white'
        });
    });

    script.addEventListener('error', e => console.log(`Can't load the Ko-fi widget`, e));

    script.type = 'text/javascript';
    script.async = true;
    script.src = 'https://storage.ko-fi.com/cdn/scripts/overlay-widget.js';

    document.body.appendChild(script);
});
