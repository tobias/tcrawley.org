var metalsmith = require('metalsmith'),
    layouts = require('metalsmith-layouts'),
    markdown = require('metalsmith-markdown'),
    jade = require('metalsmith-jade'),
    less = require('metalsmith-less'),
    serve = require('metalsmith-serve'),
    watch = require('metalsmith-watch'),
    msIf = require('metalsmith-if'),
    moment = require('moment'),
    fs = require('fs');

moment.locale('en', {
  calendar : {
    lastDay : '[Yesterday, ] MMM Do',
    sameDay : '[Today, ] MMM Do',
    lastWeek : '[last] dddd[, ] MMM Do',
    sameElse : 'll'
  }
});

build();

function build() {
  var serveAndWatch = process.argv.length > 2 && process.argv[2] === 'serve',
      metadata = JSON.parse(fs.readFileSync('./site.json', 'utf8'));

  Error.stackTraceLimit = 100;
  metadata.devMode = serveAndWatch;

  metalsmith(__dirname)
    .metadata(metadata)
    .source('./src')
    .destination('./build')

    // Write pages in markdown
    .use(markdown())
    .use(jade({useMetadata: true}))

    // use less for css
    .use(less())

    // Jade templates
    .use(layouts({
      engine: 'jade',
      moment: moment
    }))

    // when we run as `node build serve` we'll serve the site and watch
    // the files for changes. Note: This does not reload when templates
    // change, only when the content changes
    .use(msIf(
      serveAndWatch,
      watch({
        pattern: '**/*',
        livereload: false
      })))

    .use(msIf(
      serveAndWatch,
      serve({
        port: 8080,
        verbose: true
      })))

    .build(function (err) {
      if (err) {
        console.log(err);
        throw err;
      }
    });
}


