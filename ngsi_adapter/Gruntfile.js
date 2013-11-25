'use strict';

module.exports = function(grunt) {

  // Project configuration.
  grunt.initConfig({
    pkgFile: 'package.json',
    pkg: grunt.file.readJSON('package.json'),
    jshint: {
      options: {
        jshintrc: '.jshintrc'
      },
      gruntfile: {
        src: 'Gruntfile.js'
      },
      lib: {
        src: ['lib/**/*.js']
      },
      test: {
        src: ['test/**/*.js']
      }
    },
    simplemocha: {
      options: {
        ui: 'tdd',
        reporter: 'spec',
        timeout: 30000,
        ignoreLeaks: false
      },
      unit: {
        src: ['test/unit/**/*.js']
      }
    },
    exec: {
      istanbul: {
        cmd: 'node ./node_modules/.bin/istanbul cover --root lib/ -- grunt test && ' +
             'node ./node_modules/.bin/istanbul report --root target/coverage/ cobertura'
      },
      doxfoundation: {
        cmd: 'node ./node_modules/.bin/dox-foundation --source lib --target target/doc'
      }
    }
  });

  // These plugins provide necessary tasks.
  grunt.loadNpmTasks('grunt-contrib-jshint');
  grunt.loadNpmTasks('grunt-simple-mocha');
  grunt.loadNpmTasks('grunt-exec');

  // Available tasks.
  grunt.registerTask('test', ['simplemocha']);
  grunt.registerTask('coverage', ['exec:istanbul']);
  grunt.registerTask('doc', ['exec:doxfoundation']);

  // Default task.
  grunt.registerTask('default', ['jshint', 'test']);

};
