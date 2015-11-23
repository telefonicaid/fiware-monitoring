'use strict';
/* jshint -W106 */


/**
 * Grunt tasks definitions
 *
 * @param {Object} grunt
 */
module.exports = function(grunt) {

    require('load-grunt-tasks')(grunt);

    grunt.initConfig({

        pkgFile: 'package.json',
        pkg: grunt.file.readJSON('package.json'),

        dirs: {
            lib: ['lib'],
            test: ['test/unit'],
            reportTest: ['report/test'],
            reportLint: ['report/lint'],
            reportCoverage: ['report/coverage'],
            siteComplexity: ['site/complexity'],
            siteDoc: ['site/doc']
        },

        clean: {
            reportTest: ['<%= dirs.reportTest[0] %>'],
            reportLint: ['<%= dirs.reportLint[0] %>'],
            reportCoverage: ['<%= dirs.reportCoverage[0] %>'],
            siteComplexity: ['<%= dirs.siteComplexity[0] %>'],
            siteDoc: ['<%= dirs.siteDoc[0] %>']
        },

        mkdir: {
            reportTest: {
                options: {
                    create: ['<%= dirs.reportTest[0] %>']
                }
            },
            reportLint: {
                options: {
                    create: ['<%= dirs.reportLint[0] %>']
                }
            },
            reportCoverage: {
                options: {
                    create: ['<%= dirs.reportCoverage[0] %>']
                }
            },
            siteDoc: {
                options: {
                    create: ['<%= dirs.siteDoc[0] %>']
                }
            },
            siteComplexity: {
                options: {
                    create: ['<%= dirs.siteComplexity[0] %>']
                }
            }
        },

        jshint: {
            options: {
                jshintrc: '.jshintrc'
            },
            gruntfile: {
                src: 'Gruntfile.js'
            },
            lib: {
                src: ['<%= dirs.lib[0] %>/**/*.js']
            },
            test: {
                src: ['<%= dirs.test[0] %>/*.js']
            },
            reportGruntfile: {
                src: 'Gruntfile.js',
                options: {
                    reporter: 'checkstyle',
                    reporterOutput: '<%= dirs.reportLint[0] %>/jshint-gruntfile.xml'
                }
            },
            reportLib: {
                src: '<%= dirs.lib[0] %>/**/*.js',
                options: {
                    reporter: 'checkstyle',
                    reporterOutput: '<%= dirs.reportLint[0] %>/jshint-lib.xml'
                }
            },
            reportTest: {
                src: '<%= dirs.test[0] %>/*.js',
                options: {
                    reporter: 'checkstyle',
                    reporterOutput: '<%= dirs.reportLint[0] %>/jshint-test.xml'
                }
            }
        },

        watch: {
            gruntfile: {
                files: '<%= jshint.gruntfile.src %>',
                tasks: ['jshint:gruntfile']
            },
            lib: {
                files: '<%= jshint.lib.src %>',
                tasks: ['jshint:lib', 'test']
            },
            test: {
                files: '<%= jshint.test.src %>',
                tasks: ['jshint:test', 'test']
            }
        },

        env: {
            testReport: {
                NODE_ENV: 'production',
                XUNIT_FILE: '<%= dirs.reportTest[0] %>/TEST-xunit.xml'
            }
        },

        mochaTest: {
            unit: {
                options: {
                    ui: 'tdd',
                    reporter: 'spec',
                    timeout: 30000,
                    ignoreLeaks: false
                },
                src: [
                    '<%= jshint.test.src %>'
                ]
            },
            unitReport: {
                options: {
                    ui: 'tdd',
                    reporter: 'xunit-file',
                    quiet: true
                },
                src: [
                    '<%= jshint.test.src %>'
                ]
            }
        },

        mocha_istanbul: {
            coverage: {
                src: '<%= dirs.test[0] %>',
                options: {
                    root: '<%= dirs.lib[0] %>',
                    coverageFolder: '<%= dirs.reportCoverage[0] %>',
                    reportFormats: ['lcovonly']
                }
            },
            coverageReport: {
                src: '<%= dirs.test[0] %>',
                options: {
                    quiet: true,
                    root: '<%= dirs.lib[0] %>',
                    coverageFolder: '<%= dirs.reportCoverage[0] %>',
                    reportFormats: ['lcov', 'cobertura']
                }
            }
        },

        dox: {
            options: {
                title: 'NGSI Adapter Documentation'
            },
            files: {
                src: ['<%= jshint.lib.src %>', '<%= jshint.test.src %>'],
                dest: '<%= dirs.siteDoc[0] %>'
            }
        },

        plato: {
            options: {
                jshint: grunt.file.readJSON('.jshintrc')
            },
            lib: {
                files: {
                    '<%= dirs.siteComplexity[0] %>': ['<%= jshint.lib.src %>', '<%= jshint.test.src %>']
                }
            }
        },

        gjslint: {
            options: {
                reporter: {
                    name: 'console'
                },
                flags: [
                    '--flagfile .gjslintrc' //use flag file
                ],
                force: false
            },
            gruntfile: {
                src: '<%= jshint.gruntfile.src %>'
            },
            lib: {
                src: '<%= jshint.lib.src %>'
            },
            test: {
                src: '<%= jshint.test.src %>'
            },
            report: {
                options: {
                    reporter: {
                        name: 'gjslint_xml',
                        dest: '<%= dirs.reportLint[0] %>/gjslint.xml'
                    },
                    flags: [
                        '--flagfile .gjslintrc'
                    ],
                    force: false
                },
                src: ['<%= jshint.gruntfile.src %>', '<%= jshint.lib.src %>', '<%= jshint.test.src %>']
            }
        }
    });

    var mochaTestOptions = grunt.config('mochaTest.unit.options');
    var mochaTestOptionsArray = Object.keys(mochaTestOptions).map(function (opt) {
        return ('--' + opt + ',' + this[opt]).replace(/--ignoreLeaks,false/, '--check-leaks');
    }, mochaTestOptions).join().split(',');
    grunt.config('mocha_istanbul.coverage.options.mochaOptions', mochaTestOptionsArray);
    grunt.config('mocha_istanbul.coverageReport.options.mochaOptions', mochaTestOptionsArray);

    grunt.registerTask('test', 'Run tests',
        ['mochaTest:unit']);

    grunt.registerTask('test-report', 'Generate tests reports',
        ['env', 'clean:reportTest', 'mkdir:reportTest', 'mochaTest:unitReport']);

    grunt.registerTask('coverage', 'Print coverage summary',
        ['mocha_istanbul:coverage']);

    grunt.registerTask('coverage-report', 'Generate coverage reports',
        ['mocha_istanbul:coverageReport']);

    grunt.registerTask('complexity', 'Generate code complexity reports',
        ['mkdir:siteComplexity', 'plato']);

    grunt.registerTask('doc', 'Generate source code JSDoc',
        ['mkdir:siteDoc', 'dox']);

    grunt.registerTask('lint-jshint', 'Check source code style with JsHint',
        ['jshint:gruntfile', 'jshint:lib', 'jshint:test']);

    grunt.registerTask('lint-gjslint', 'Check source code style with Google Closure Linter',
        ['gjslint:gruntfile', 'gjslint:lib', 'gjslint:test']);

    grunt.registerTask('lint', 'Check source code style',
        ['lint-jshint', 'lint-gjslint']);

    grunt.registerTask('lint-report', 'Generate checkstyle reports',
        ['clean:reportLint', 'mkdir:reportLint', 'jshint:reportGruntfile', 'jshint:reportLib',
        'jshint:reportTest', 'gjslint:report']);

    grunt.registerTask('site',
        ['doc', 'complexity']);

    // Default task.
    grunt.registerTask('default',
        ['lint-jshint', 'test']);

};
